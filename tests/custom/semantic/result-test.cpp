#include "../../../src/frontend/frontend.h"
#include "../../../src/semantic/analyzer.h"
#include <set>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace {
using namespace rx;
using namespace rx::semantic;

void require(bool is_valid, const std::string& message) {
  if (!is_valid) {
    throw std::runtime_error(message);
  }
}

FrontendResult compile(const std::string& text, Stage stage = Stage::SEMANTIC) {
  std::istringstream source(text);
  return check(source, "test.rx", stage);
}

const AnalysisResult& accepted(const FrontendResult& frontend) {
  for (const auto& diagnostic : frontend.diagnostics) {
    std::cerr << diagnostic.line << ':' << diagnostic.column << ' '
              << diagnostic.message << '\n';
  }
  require(frontend.diagnostics.empty(), "expected acceptance");
  require(frontend.semantic != nullptr, "missing semantic result");
  return *frontend.semantic;
}

const ast::FunctionItem* function(const AnalysisResult& result,
  const std::string& name) {
  for (const auto& entry : result.functions) {
    if (entry.first->name == name) {
      return entry.first;
    }
  }
  throw std::runtime_error("missing function " + name);
}

const ast::LetStmt* local(const AnalysisResult& result, const std::string& name) {
  for (const auto& entry : result.locals) {
    if (entry.first->name == name) {
      return entry.first;
    }
  }
  throw std::runtime_error("missing local " + name);
}

void testBindings() {
  auto frontend = compile(R"(
    fn pick(arg: i32) -> i32 {
      let x = arg;
      { let x = true; let inner = x; }
      let outer = x;
      return outer;
    }
    fn main() { let value = pick(1); println_i32(value); }
  )");
  const auto& result = accepted(frontend);
  const auto* inner = local(result, "inner");
  const auto* outer = local(result, "outer");
  const auto& inner_path = std::get<ast::PathExpr>(inner->init->value);
  const auto& outer_path = std::get<ast::PathExpr>(outer->init->value);
  const auto* inner_x = std::get<const ast::LetStmt*>(result.bindings.at(&inner_path));
  const auto* outer_x = std::get<const ast::LetStmt*>(result.bindings.at(&outer_path));
  require(inner_x != outer_x, "shadowed bindings must have distinct identities");
  require(result.locals.at(inner_x).type->kind == TypeKind::BOOL, "inner type");
  require(result.locals.at(outer_x).type->kind == TypeKind::I32, "outer type");
  const auto* pick = function(result, "pick");
  const auto& argument = std::get<ast::PathExpr>(outer_x->init->value);
  require(std::get<const ast::Parameter*>(result.bindings.at(&argument))
    == &pick->parameters.front(), "parameter binding");
  require(result.functions.at(pick).result->kind == TypeKind::I32, "return type");
  require(result.returns.begin()->second == pick, "return target");
  require(result.calls.size() == 2, "call resolutions");
  const auto* value = local(result, "value");
  const auto& call = std::get<ast::CallExpr>(value->init->value);
  require(result.calls.at(&call).function == pick, "function target");
  require(result.expressions.at(outer->init.get()).place.has_value(), "read place");
}

void testTypesAndConstants() {
  auto frontend = compile(R"(
    const SIZE: usize = 3;
    const NEG: i32 = -7;
    struct Pair { first: i32, second: bool }
    impl Pair {
      const COUNT: usize = SIZE;
      fn make() -> Self { Self { second: true, first: NEG } }
    }
    fn main() {
      let a: [i32; SIZE] = [1; Pair::COUNT];
      let pair = Box::<Pair>::new(Pair::make());
      let field = pair.second;
      let index = a[0];
    }
  )");
  const auto& result = accepted(frontend);
  require(result.constants.size() == 3, "constant declarations");
  const auto* a = local(result, "a");
  require(result.locals.at(a).type->length == 3, "evaluated array length");
  const auto& repeat = std::get<ast::RepeatExpr>(a->init->value);
  require(result.constant_expressions.at(repeat.length.get()).value == 3,
    "associated constant evaluation");
  const auto* pair = result.structs.at("Pair").declaration;
  const auto* make = function(result, "make");
  require(result.functions.at(make).result->declaration == pair, "resolved Self");
  const auto* field = local(result, "field");
  const auto& access = std::get<ast::FieldExpr>(field->init->value);
  require(result.fields.at(&access).index == 1, "field declaration order");
  require(result.fields.at(&access).owner == pair, "field owner");
  require(result.expressions.at(access.object.get()).adjustments.size() == 1,
    "field auto dereference");
  require(result.initializers.size() == 2, "initializer fields");
  for (const auto& entry : result.initializers) {
    require(pair->fields[entry.second.index].name == entry.first->name,
      "initializer must use declaration order");
  }
  std::set<const Type*> owned;
  for (const Type& type : result.types) {
    owned.insert(&type);
  }
  for (const Type& type : result.types) {
    require(type.target == nullptr || owned.count(type.target), "owned child type");
  }
  for (const auto& entry : result.expressions) {
    require(owned.count(entry.second.type), "owned expression type");
  }
  const Type* saved = result.locals.at(a).type;
  auto moved = std::move(frontend);
  require(moved.semantic->locals.at(a).type == saved, "stable type after move");
  Analyzer analyzer;
  std::vector<Diagnostic> diagnostics;
  const auto& crate = *std::get<std::unique_ptr<ast::Crate>>(*moved.tree);
  auto first = analyzer.run(crate, "test.rx", diagnostics);
  const Type* first_type = first->locals.at(a).type;
  auto second = analyzer.run(crate, "test.rx", diagnostics);
  require(second != nullptr && first_type->length == 3, "analyzer reuse lifetime");
}

void testMethods() {
  auto frontend = compile(R"(
    #[derive(Clone)]
    struct Counter { value: i32 }
    impl Counter {
      fn read(&self) -> i32 { self.value }
      fn add(&mut self, n: i32) { self.value += n; }
    }
    fn main() {
      let mut boxed = Box::<Counter>::new(Counter { value: 1 });
      let read = boxed.read();
      boxed.add(2);
      let mut vector = Vec::<i32>::new();
      vector.push(read);
      let length = vector.len();
      let copied = boxed.clone();
    }
  )");
  const auto& result = accepted(frontend);
  require(result.methods.size() == 5, "all method resolutions");
  const auto* read = local(result, "read");
  const auto& call = std::get<ast::MethodCallExpr>(read->init->value);
  const auto& info = result.methods.at(&call);
  require(info.function == function(result, "read"), "inherent method target");
  require(info.receiver_adjustments.size() == 2, "dereference then borrow");
  require(info.receiver_adjustments[0].kind == AdjustmentKind::DEREFERENCE,
    "receiver dereference");
  require(info.receiver_adjustments[1].kind == AdjustmentKind::BORROW,
    "receiver shared borrow");
  bool has_push = false;
  bool has_mutable_method = false;
  for (const auto& entry : result.methods) {
    if (entry.second.builtin == Builtin::PUSH) {
      has_push = true;
      require(entry.second.receiver->is_mutable, "mutable Vec receiver");
    }
    if (entry.second.function == function(result, "add")) {
      has_mutable_method = true;
      require(entry.second.receiver_adjustments.back().kind
        == AdjustmentKind::BORROW_MUT, "mutable inherent receiver");
    }
  }
  require(has_push && has_mutable_method, "mutable method resolutions");
  const auto* length = local(result, "length");
  const auto& len_call = std::get<ast::MethodCallExpr>(length->init->value);
  require(result.methods.at(&len_call).builtin == Builtin::LEN, "builtin target");
}

void testAdjustmentsAndLoops() {
  auto frontend = compile(R"(
    fn read(x: &i32) -> i32 { *x }
    fn main() {
      let mut value = 1;
      let shared: &i32 = &mut value;
      let boxed = Box::<i32>::new(2);
      let read = read(&boxed);
      let branch = if true { &mut value } else { &value };
      let array = [&mut value, &value];
      let exit = loop {
        if true { break &mut value; }
        break &value;
      };
      loop { while false { continue; } break; }
      let sum = &value + &value;
      let nested = loop { break loop { break 7; }; };
    }
  )");
  const auto& result = accepted(frontend);
  const auto* shared = local(result, "shared");
  const auto& conversion = result.expressions.at(shared->init.get());
  require(conversion.type->is_mutable, "preserve original expression type");
  require(conversion.adjustments.size() == 2, "mutable to shared reborrow");
  require(!conversion.adjustments.back().target->is_mutable, "coerced type");
  const auto* read = local(result, "read");
  const auto& call = std::get<ast::CallExpr>(read->init->value);
  require(result.expressions.at(call.arguments.front().get()).adjustments.size()
    == 3, "reference and Box dereference then borrow");
  const auto* branch = local(result, "branch");
  const auto& conditional = std::get<ast::IfExpr>(branch->init->value);
  require(result.expressions.at(conditional.then_branch.get()).adjustments.size()
    == 2, "branch common type conversion");
  const auto* array = local(result, "array");
  const auto& elements = std::get<ast::ArrayExpr>(array->init->value).elements;
  require(result.expressions.at(elements.front().get()).adjustments.size() == 2,
    "array common type conversion");
  const auto* exit = local(result, "exit");
  const auto& loop = std::get<ast::LoopExpr>(exit->init->value);
  std::size_t loop_breaks = 0;
  for (const auto& entry : result.breaks) {
    const auto* target = std::get_if<const ast::LoopExpr*>(&entry.second);
    if (target != nullptr && *target == &loop) {
      ++loop_breaks;
      const auto& value = result.expressions.at(entry.first->value.get());
      if (value.type->is_mutable) {
        require(value.adjustments.size() == 2, "break common type conversion");
      }
    }
  }
  require(loop_breaks == 2, "break targets");
  require(std::holds_alternative<const ast::WhileExpr*>(
    result.continues.begin()->second), "continue targets inner loop");
  require(result.locals.at(local(result, "nested")).type->kind == TypeKind::I32,
    "nested break value keeps outer loop context");
  const auto* sum = local(result, "sum");
  const auto& binary = std::get<ast::BinaryExpr>(sum->init->value);
  require(result.expressions.at(binary.left.get()).adjustments.size() == 1,
    "operator implicit dereference");
}

void testNestedReturnCoercion() {
  auto frontend = compile(R"(
    fn unwrap(value: &Box<i32>) -> &i32 { { value } }
    fn main() { let b = Box::<i32>::new(1); let r = unwrap(&b); }
  )");
  const auto& result = accepted(frontend);
  const auto* unwrap = function(result, "unwrap");
  const auto& outer = std::get<ast::BlockExpr>(unwrap->body->value);
  const auto& inner = std::get<ast::BlockExpr>(outer.tail->value);
  require(result.expressions.at(inner.tail.get()).adjustments.size() == 3,
    "apply nested return coercion to the value");
  require(result.expressions.at(outer.tail.get()).adjustments.empty(),
    "inner block must not repeat tail coercion");
  const auto& body = result.expressions.at(unwrap->body.get());
  require(body.adjustments.empty(), "function body must not repeat coercion");
  require(sameType(body.type, result.functions.at(unwrap).result),
    "block type includes tail adjustment");
}

void testFailureAndStages() {
  auto failed = compile("fn main() { let x: bool = 1; }");
  require(!failed.diagnostics.empty() && failed.semantic == nullptr,
    "invalid program must not expose a lowering result");
  auto parsed = compile("fn main() {}", Stage::PARSE);
  require(parsed.diagnostics.empty() && parsed.semantic == nullptr,
    "parse stage must not analyze");
  auto ast = compile("fn main() {}", Stage::AST);
  require(ast.tree.has_value() && ast.semantic == nullptr,
    "AST stage must not analyze");
}
}

int main() {
  try {
    testBindings();
    testTypesAndConstants();
    testMethods();
    testAdjustmentsAndLoops();
    testNestedReturnCoercion();
    testFailureAndStages();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  std::cout << "semantic result tests passed\n";
  return 0;
}
