# Parser conventions

The language grammar is defined by the Lexer and Syntax blocks in each chapter.
The [grammar summary](grammar-summary.md) collects those same productions in
one place. This guide indexes the rules for precedence and expression boundaries and
specifies contextual token interpretation.

<details>
<summary>Grammar coverage by source form</summary>

Each occurrence of [Type] or [Expression] admits all alternatives defined in its
respective production. This composition applies recursively: a struct field
may hold a nested container type such as `Vec<Box<T>>`, a constructor argument may contain a block, and a
constructor result may be chained with indexing or a method call.

| Source form | Defining productions | Examples |
| --- | --- | --- |
| Identifiers and integers | [IDENTIFIER], [INTEGER_LITERAL] | `_value`, `123_i32`, `0b1010` |
| Lifetime tokens and forms | [LIFETIME_TOKEN], [Lifetime] | `'a`, `'static`, `'_` |
| Lifetime parameters and bounds | [GenericParams], [LifetimeParam], [WhereClause] | `fn f<'a>(x: &'a i32)`, `where 'a: 'b` |
| Source compilation unit | [Crate], [Item] | Top-level use declarations, functions, structs, constants, impls |
| Use declarations | [UseDeclaration], [UseTree], [UsePath] | `use rx::core::*;`, `use rx::core::{get_i32, println_i32};` |
| Functions and receivers | [Function], [FunctionParameters], [ShorthandSelf] | `fn f(v: Vec<i32>) -> Box<i32> { ... }`, `&mut self` |
| Struct declarations and construction | [StructStruct], [StructExpression] | `struct S { x: i32 }`, `S { x: 1 }`, `Self { x: 1 }` |
| Associated items | [InherentImpl], [AssociatedItem] | `impl S { fn new() -> Self { ... } }` |
| Derive attributes | [OuterAttribute], [DeriveAttribute] | `#[derive(Clone, PartialEq, Eq)]` |
| Bindings and statements | [LetStatement], [ExpressionStatement] | `let mut v = Vec::<i32>::new();`, `v.push(1);` |
| Type names, arrays, references, and unit | [TypePath], [ArrayType], [ReferenceType], [UnitType] | `Self`, `[Vec<i32>; 2]`, `&mut Box<i32>`, `()` |
| Container types | [TypePath], [TypePathSegment], [GenericArgs] | `Box<[i32; 4]>`, `Vec<Box<Node>>`, `Vec<Vec<i32,>,>` |
| Container construction | [CallExpression], [PathInExpression] | `Box::<i32>::new(7)`, `Vec::<Box<i32>>::new()` |
| Paths, calls, fields, and indexing | [PathExpression], [CallExpression], [MethodCallExpression], [FieldExpression], [IndexExpression] | `S::new()`, `value.clone()`, `value.field`, `values[0]` |
| Arrays, grouping, and unit values | [ArrayExpression], [GroupedExpression], [UnitExpression] | `[1, 2, 3]`, `[0; 4]`, `(value)`, `()` |
| Operators | [OperatorExpression] | `*p`, `&mut values[0]`, `a + b`, `x += &y` |
| Blocks and branching | [BlockExpression], [IfExpression] | `{ make() }.field`, `if flag { a } else { b }` |
| Loops and jumps | [LoopExpression], [BreakExpression], [ContinueExpression], [ReturnExpression] | `loop { break 3; }`, `while condition { ... }`, `return value` |
| Constant contexts | [ConstValue], [Magnitude] | `123`, `COUNT`, `(-1)`, `-(2147483648i32)`, `((true))` |

The grammar describes the shape of a construct. Name resolution, typing, place
mutability, and the [test domain](undefined-behavior.md) determine which parsed
programs are valid. For example, the identifier in a struct construction expression must
name a declared struct, a called method must be defined in an inherent impl for the receiver's type,
and an assignment destination must denote a mutable place.

</details>

<details>
<summary>Rule locations</summary>

| Topic | Defining chapter |
| --- | --- |
| Use declarations | [Use declarations](items/use-declarations.md) |
| Type and expression arguments | [Paths](paths.md#generic-arguments) |
| Lifetime declarations and bounds | [Lifetime parameters](items/generics.md) |
| Derive attributes | [Attribute grammar](traits-and-attributes.md#attribute-grammar) |
| Operator precedence | [Expressions](expressions.md#precedence) |
| Casts followed by comparison or shift | [Cast parsing](expressions/operator-expr.md#cast-parsing) |
| Struct construction in a condition | [If expressions](expressions/if-expr.md) |
| Expression-statement boundaries | [Statements](statements.md#statement-boundary) |
| Block tails | [Block expressions](expressions/block-expr.md) |
| Break/continue targets and loop results | [Loops](expressions/loop-expr.md#while-and-loop) |
| Never and unreachable code | [Never type](types/never.md) |
| Assignment places | [Assignment destinations](expressions/operator-expr.md#assignment-destinations) |

</details>

## Syntax that may be discarded after parsing

Two forms of Rust compatibility syntax must be parsed, but need no subsequent
semantic processing or LLVM IR representation:

- [Use declarations](items/use-declarations.md): discard the entire declaration.
  No import resolution, module loading, alias insertion, or import-conflict
  checking is required.
- Lifetime syntax: discard lifetime parameter declarations, reference lifetime
  annotations, explicit lifetime arguments, outlives bounds, and lifetime
  where clauses. No lifetime name resolution, inference, elision checking, or
  lifetime/borrow checker is required.

The [use compatibility](undefined-behavior.md#use-compatibility) and
[lifetime validity](undefined-behavior.md#lifetime-validity) guarantees exclude
tests that would require these checks. Discarding lifetime syntax is an
internal AST transformation; the compiler is not required to ensure that source
text stripped of lifetime annotations forms valid Rust.

Parsing must still enforce the grammar of these constructs. The compiler must preserve
reference types, mutability qualifiers, concrete type arguments (such as `i32` in `Vec::<&'a i32>`),
and all other AST structures. Ordinary name, type, and place-mutability
checks and correct reference behavior remain required.

## Contextual punctuation

The parser interprets combined punctuation according to its current grammar
context. The lexer may emit a combined token; the parser consumes the needed
prefix and leaves the remaining punctuation for the enclosing construct.

| Combined token | Context | Interpretation |
| --- | --- | --- |
| `&&` | Reference type or prefix borrow | Two `&` tokens, as in `&&i32` or `&&x` |
| `>>` | Closing nested type arguments | Two closing `>` tokens |
| `>=` | Closing type arguments before assignment | A closing `>` followed by `=` |
| `>>=` | Closing nested type arguments before assignment | Two closing `>` tokens followed by `=` |

In ordinary infix expression contexts, these tokens denote their corresponding
operators. Thus, the following variable initializations do not require whitespace between the closing `>` and the `=` token:

```rust,ignore
let values: Vec<i32>=Vec::<i32>::new();
let nested: Vec<Vec<i32>>=Vec::<Vec<i32>>::new();
```
