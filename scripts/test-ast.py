#!/usr/bin/env python3
"""Test AST construction, normalized structure, source spans, and error gating."""

import argparse
from bisect import bisect_right
from collections import Counter
import json
import math
from pathlib import Path
import runpy
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
loadManifest = runpy.run_path(str(ROOT / "scripts/test-frontend.py"))["loadManifest"]

def checkSubset(actual, expected, path="root"):
  if isinstance(expected, dict):
    if not isinstance(actual, dict):
      raise AssertionError(f"{path}: expected an object")
    for key, value in expected.items():
      if key not in actual:
        raise AssertionError(f"{path}: missing {key}")
      checkSubset(actual[key], value, f"{path}.{key}")
  elif isinstance(expected, list):
    if not isinstance(actual, list) or len(actual) != len(expected):
      raise AssertionError(f"{path}: expected list of length {len(expected)}")
    for index, value in enumerate(expected):
      checkSubset(actual[index], value, f"{path}[{index}]")
  elif actual != expected:
    raise AssertionError(f"{path}: expected {expected!r}, got {actual!r}")

def checkSpans(root, source):
  line_starts = [0] + [index + 1 for index, char in enumerate(source) if char == "\n"]
  pending = [(root, 0, len(source))]
  while pending:
    node, begin, end = pending.pop()
    if isinstance(node, dict):
      if "span" in node:
        span = node["span"]
        if not begin <= span["begin"] <= span["end"] <= end:
          raise AssertionError(f"invalid/nonnested span: {span}, parent [{begin}, {end})")
        begin, end = span["begin"], span["end"]
        line = bisect_right(line_starts, begin)
        if span["line"] != line or span["column"] != begin - line_starts[line - 1] + 1:
          raise AssertionError(f"wrong source location: {span}")
      pending.extend((value, begin, end) for key, value in node.items() if key != "span")
    elif isinstance(node, list):
      pending.extend((value, begin, end) for value in node)

def withoutSpans(node):
  if isinstance(node, dict):
    return {key: withoutSpans(value) for key, value in node.items() if key != "span"}
  if isinstance(node, list):
    return [withoutSpans(value) for value in node]
  return node

class AstRunner:
  def __init__(self, compiler, timeout):
    self.compiler = compiler
    self.timeout = timeout
    self.total = Counter()
    self.failed = Counter()

  def run_case(self, group, source, entry="crate", should_accept=True, expected=None):
    self.total[group] += 1
    try:
      result = subprocess.run(
        [str(self.compiler), "--stage", "ast", "--entry", entry, str(source)],
        capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=self.timeout,
      )
      if not should_accept:
        if result.returncode != 1 or result.stdout or not result.stderr:
          raise AssertionError(f"rejected source must return 1, diagnose, and emit no AST; got {result.returncode}")
        return None
      if result.returncode != 0 or result.stderr:
        raise AssertionError(f"exit {result.returncode}: {result.stderr}")
      document = json.loads(result.stdout)
      if document["file"] != str(source) or document["span_units"] != "unicode-codepoints":
        raise AssertionError("wrong AST document metadata")
      root = document["root"]
      checkSpans(root, source.read_bytes().decode("utf-8"))
      if expected is not None:
        checkSubset(root, expected)
      return root
    except (AssertionError, ValueError, KeyError, TypeError, subprocess.TimeoutExpired) as error:
      self.failed[group] += 1
      print(f"FAIL {group}: {source.name}: {error}")
      return None

  def summary(self):
    for group, count in self.total.items():
      print(f"{group}: {count - self.failed[group]}/{count} passed")
    print(f"Total: {sum(self.total.values()) - sum(self.failed.values())} passed, {sum(self.failed.values())} failed")
    return 1 if self.failed else 0

def officialTests(runner, official):
  for suite in ("parser", "semantic", "codegen", "optimization"):
    manifests = sorted((official / suite).rglob("manifest.json"))
    if not manifests:
      raise ValueError(f"no manifests in {official / suite}")
    for manifest in manifests:
      for row in loadManifest(manifest):
        entry = "crate"
        should_accept = True
        if suite == "parser":
          entry = row.get("metadata", {}).get("entry", "crate")
          should_accept = row["compilation_success"]
        runner.run_case(f"official:{suite}", manifest.parent / row["source"], entry, should_accept)
  manifest = official / "lexer/manifest.json"
  for row in loadManifest(manifest):
    if not row["compilation_success"]:
      runner.run_case("official:lex-errors", manifest.parent / row["source"], should_accept=False)

def customTests(runner):
  cases = json.loads((ROOT / "tests/custom/ast/cases.json").read_text())
  if not cases:
    raise ValueError("empty custom AST suite")
  with tempfile.TemporaryDirectory(prefix="rx-ast-") as directory:
    directory = Path(directory)
    for case in cases:
      source = directory / f"{case['name']}.rx"
      source.write_text(case["source"], encoding="utf-8")
      runner.run_case("custom:structure", source, case["entry"], expected=case["expect"])

    for index, op in enumerate(("+", "-", "*", "/", "%", "&", "|", "^", "<<", ">>",
      "==", "!=", "<", "<=", ">", ">=", "&&", "||", "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=")):
      source = directory / f"operator-{index}.rx"
      source.write_text(f"a {op} b")
      kind = "AssignExpr" if index >= 18 else "BinaryExpr"
      runner.run_case("custom:operators", source, "expression", expected={"kind": kind, "op": op})
    for index, op in enumerate(("-", "!", "*", "&", "&mut")):
      source = directory / f"unary-{index}.rx"
      source.write_text(f"{op} x")
      runner.run_case("custom:operators", source, "expression", expected={"kind": "UnaryExpr", "op": op})

    templates = (
      ("{}", []),
      ("if {} {{}}", ["condition"]),
      ("if break ({}) {{}}", ["condition", "value"]),
      ("{{ {}; }}", ["statements", 0, "expression"]),
      ("{{ {} }}", ["tail"]),
      ("{{ let x = {}; }}", ["statements", 0, "init"]),
    )
    for index, expression in enumerate(("a - b - c + d * e", "a << b >> c", "a as (u32) < b",
      "a as Vec::<i32> << b", "a | b ^ c & d", "a = b += c", "a >= b && c <= d", "&&mut a")):
      baseline = None
      for position, (template, keys) in enumerate(templates):
        source = directory / f"family-{index}-{position}.rx"
        source.write_text(template.format(expression))
        root = runner.run_case("custom:expression-families", source, "expression")
        if root is None:
          continue
        for key in keys:
          root = root[key]
        root = withoutSpans(root)
        if position == 0:
          baseline = root
        elif baseline != root:
          runner.failed["custom:expression-families"] += 1
          print(f"FAIL expression family normalization: {source.name}")

    for index, (text, entry) in enumerate((("fn f() { let x = 1 }", "crate"),
      ("1 2", "expression"), ("i32 i32", "typeRef"), ("fn a() {} fn b() {}", "item"),
      ("let x = 1; let y = 2;", "letStatement"), ("fn f() {} @", "crate"))):
      source = directory / f"no-ast-on-error-{index}.rx"
      source.write_text(text)
      runner.run_case("custom:error-gating", source, entry, should_accept=False)
    source = directory / 'quoted-"name.rx'
    source.write_text("fn f() {}")
    runner.run_case("custom:json-escaping", source)

def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument("--compiler", type=Path, default=ROOT / "target/frontend/rxc")
  parser.add_argument("--official", type=Path, default=ROOT / "tests/official")
  parser.add_argument("--timeout", type=float, default=20)
  args = parser.parse_args()
  if not math.isfinite(args.timeout) or args.timeout <= 0:
    parser.error("--timeout must be finite and positive")
  runner = AstRunner(args.compiler.resolve(), args.timeout)
  try:
    officialTests(runner, args.official.resolve())
    customTests(runner)
    return runner.summary()
  except (OSError, ValueError, KeyError, TypeError) as error:
    print(f"ERROR: {error}", file=sys.stderr)
    return 2

if __name__ == "__main__":
  sys.exit(main())
