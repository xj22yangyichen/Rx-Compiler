#!/usr/bin/env python3
"""Check lex/parse acceptance and diagnostics without running semantic analysis."""

import argparse
from collections import Counter
import json
import math
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def loadManifest(path):
  rows = json.loads(path.read_text(encoding="utf-8"))
  if not isinstance(rows, list) or not rows:
    raise ValueError(f"empty or invalid manifest: {path}")
  for row in rows:
    if not isinstance(row.get("compilation_success"), bool):
      raise ValueError(f"invalid compilation_success in {path}")
    source = path.parent / row["source"]
    if not source.is_file():
      raise ValueError(f"missing source: {source}")
  return rows

class Runner:
  def __init__(self, compiler, timeout):
    self.compiler = compiler
    self.timeout = timeout
    self.total = Counter()
    self.failed = Counter()

  def run(self, group, label, arguments, expected, contains=(), excludes=(),
    diagnostic_count=None):
    self.total[group] += 1
    problems = []
    try:
      result = subprocess.run(
        [str(self.compiler), *map(str, arguments)],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=self.timeout,
      )
    except subprocess.TimeoutExpired:
      problems.append(f"timeout after {self.timeout:g}s")
    else:
      if result.returncode != expected:
        problems.append(f"expected exit {expected}, got {result.returncode}")
      if result.stdout:
        problems.append("unexpected stdout")
      if expected == 0 and result.stderr:
        problems.append("unexpected stderr on success")
      if expected != 0 and not result.stderr:
        problems.append("missing diagnostic on stderr")
      for fragment in contains:
        if fragment not in result.stderr:
          problems.append(f"missing diagnostic fragment: {fragment!r}")
      for fragment in excludes:
        if fragment in result.stderr:
          problems.append(f"unexpected diagnostic fragment: {fragment!r}")
      if diagnostic_count is not None and len(result.stderr.splitlines()) != diagnostic_count:
        problems.append(f"expected {diagnostic_count} diagnostic lines")
      if problems and result.stderr:
        problems.append(result.stderr.rstrip())

    if problems:
      self.failed[group] += 1
      print(f"FAIL {group}: {label}\n  " + "\n  ".join(problems))

  def summary(self):
    for group, count in self.total.items():
      print(f"{group}: {count - self.failed[group]}/{count} passed")
    total = sum(self.total.values())
    failed = sum(self.failed.values())
    print(f"Total: {total - failed} passed, {failed} failed")
    return 1 if failed else 0

def officialTests(runner, official):
  for suite, stage in (("lexer", "lex"), ("parser", "parse")):
    manifest = official / suite / "manifest.json"
    for row in loadManifest(manifest):
      if row["stage"] != stage:
        raise ValueError(f"unexpected stage in {manifest}: {row['stage']}")
      arguments = ["--stage", stage]
      if stage == "parse":
        arguments += ["--entry", row.get("metadata", {}).get("entry", "crate")]
      arguments.append(manifest.parent / row["source"])
      runner.run(f"official:{stage}", row["source"], arguments,
        0 if row["compilation_success"] else 1)

def parseRegressions(runner, official):
  for suite in ("semantic", "codegen", "optimization"):
    manifests = sorted((official / suite).rglob("manifest.json"))
    if not manifests:
      raise ValueError(f"no manifests found under {official / suite}")
    for manifest in manifests:
      for row in loadManifest(manifest):
        # Semantic rejection does not imply a syntax error.
        runner.run(f"parse-only:{suite}", f"{manifest.parent.name}/{row['source']}",
          ["--stage", "parse", manifest.parent / row["source"]], 0)

def customTests(runner):
  cases = json.loads((ROOT / "tests/custom/frontend/cases.json").read_text(encoding="utf-8"))
  if not cases:
    raise ValueError("no custom frontend cases found")
  with tempfile.TemporaryDirectory(prefix="rx-frontend-") as directory:
    directory = Path(directory)
    for case in cases:
      source = directory / f"{case['name']}.rx"
      source.write_text(case["source"], encoding="utf-8")
      arguments = ["--stage", case["stage"]]
      if "entry" in case:
        arguments += ["--entry", case["entry"]]
      arguments.append(source)
      runner.run("custom:diagnostics", case["name"], arguments, case["exit"],
        contains=case.get("contains", []), excludes=case.get("excludes", []),
        diagnostic_count=case.get("diagnostic_count"))

def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument("--compiler", type=Path, default=ROOT / "target/frontend/rxc")
  parser.add_argument("--official", type=Path, default=ROOT / "tests/official")
  parser.add_argument("--timeout", type=float, default=10)
  parser.add_argument("--parse-regressions", action="store_true", dest="should_parse_regressions",
    help="also parse all semantic/codegen/optimization sources")
  args = parser.parse_args()
  if not math.isfinite(args.timeout) or args.timeout <= 0:
    parser.error("--timeout must be finite and positive")
  runner = Runner(args.compiler.resolve(), args.timeout)
  try:
    if not runner.compiler.is_file():
      raise ValueError(f"compiler not found: {runner.compiler}")
    officialTests(runner, args.official.resolve())
    customTests(runner)
    if args.should_parse_regressions:
      parseRegressions(runner, args.official.resolve())
  except (OSError, ValueError, KeyError, TypeError) as error:
    print(f"ERROR: {error}", file=sys.stderr)
    return 2
  return runner.summary()

if __name__ == "__main__":
  sys.exit(main())
