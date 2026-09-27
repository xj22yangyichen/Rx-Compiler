# Language scope and test guarantees

## Definition

We distinguish among valid programs, compile errors, and undefined behavior:

| Category | Compiler requirement | Test policy |
| --- | --- | --- |
| Valid program | Compile and produce the specified behavior | May appear in any applicable test |
| Compile error | Reject normally; diagnostic wording is unspecified | May appear in negative tests |
| Undefined behavior | No required diagnostic or behavior | Excluded as stated by its defining rule |

Unsupported syntax may be rejected at the language-subset boundary even if the supplied parser recognizes it. Unless a source form is explicitly classified as undefined behavior, ordinary name, type, and place-mutability errors remain compile errors, including in unreachable code. The compiler need not prove termination, borrow validity, or ownership validity. Runtime checks, panic handling, and stack unwinding are not required for excluded executions.

## Language subset

Rx is a subset of Rust 2021. Every Rx source program uses valid Rust syntax, but this specification defines its own built-in environment and required runtime behavior.

Rx includes:

- Functions, named-field structs, constants, and inherent implementations.
- Top-level [use declarations](items/use-declarations.md) for Rust compatibility, ignored after parsing.
- Identifier bindings and local type inference.
- Integers, booleans, fixed arrays, references, and builtin `Box` and `Vec` containers.
- `if`, `while`, `loop`, `return`, `break`, and `continue` control flow.
- Struct derives for `Copy`, `Clone`, `PartialEq`, and `Eq`.

The Rx subset excludes:

- User traits, trait bounds, and trait objects.
- Enums, non-unit tuples, tuple structs, tuple variants, and unit variants.
- Pattern matching, destructuring, and guards.
- Type parameters and general generics. Lifetime syntax is supported, but its validity is guaranteed rather than checked.
- Module declarations and import resolution, visibility, privacy, macros, and arbitrary attributes. Use declarations are accepted only as described by the [use compatibility guarantee](#use-compatibility).
- Unsafe Rust and the standard library beyond the builtins defined here.
- Const generics and general constant evaluation beyond the restricted [constant contexts](const_eval.md). Constant propagation remains an optional optimization.
- Integer overflow checks.

[Types](types.md), [expressions](expressions.md), and the [grammar summary](grammar-summary.md) define the complete subset. Forms outside that grammar are unsupported.

## Test guarantees

The following source forms trigger undefined behavior and will never appear in test suites, including within unreachable code or constant contexts where applicable:

| Undefined behavior | Defining rules |
| --- | --- |
| Integer literal outside its determined type | [Integer literal range](#integer-literal-range) |
| Integer inference requiring expected-type propagation through binary operations, unary operations, or shared/mutable borrows | [Expected-type propagation](types.md#coercion-sites-and-expected-types) |
| Omitting the explicit concrete type argument in a `Box::new(...)` or `Vec::new()` constructor call | [Builtin constructors](heap.md#builtin-signatures) |
| Using a function as a value | [Function values](expressions/path-expr.md#function-values) |
| Declaring a struct named `u32`, `isize`, `usize`, `bool`, `Box`, `Vec`, `Copy`, `Clone`, `PartialEq`, or `Eq` | [Protected builtin names](names.md#protected-builtin-names) |
| Local binding named `get_i32`, `print_i32`, or `println_i32` | [Protected builtin names](names.md#protected-builtin-names) |
| Place-mutability violation in unreachable code, including assignment, mutable borrowing, or mutable receiver adjustment | [Unreachable code](types/never.md#unreachable-code) |
| Array or `Vec` index operand with the never type `!` | [Index operand types](expressions/array-expr.md#index-operand-types) |
| Documentation comment (`///`, `//!`, `/**`, `/*!`) | [Documentation comments](comments.md#documentation-comments) |
| Let or parameter name colliding with a visible unqualified constant | [Constant-name collisions](#constant-name-collisions) |
| Equality between different source types | [Cross-type equality](#cross-type-equality) |
| LUB coercion for which neither type can become the common target | [Least upper bound coercions](types.md#least-upper-bound-coercions) |
| Multiple distinct matching methods in a dot call | [Method lookup](expressions/method-call-expr.md#method-lookup) |
| Use declaration violating the Rust compatibility guarantees | [Use compatibility](#use-compatibility) |
| Invalid lifetime declaration, use, bound, or elision | [Lifetime validity](#lifetime-validity) |
| Observable zero-sized data use outside the unit-result exception | [Zero-sized data](#zero-sized-data) |

These guarantees constrain valid source programs. Any invalid memory access, incorrect termination, or incorrect program output introduced during compilation remains a compiler defect.

### Integer literal range

An integer literal's type is selected by its suffix, its expected integer type, or the `i32` fallback, in that order. An unsuffixed integer literal whose value falls outside the `i32` range when evaluated without an expected type constitutes undefined behavior.

Integer literal overflow exhibits undefined behavior. The [literal rules](expressions/literal-expr.md#integer-typing-and-range) still permit signed minima such as `-2147483648i32` and `-(2147483648i32)`.

### Constant-name collisions

A `let` binding or function parameter whose name matches a visible unqualified constant triggers undefined behavior, regardless of mutability or declaration order. No diagnostic or constant-pattern interpretation is required. An associated constant reachable only as `Type::NAME` does not exclude a local `NAME`. Collisions with builtin names follow the [protected builtin rules](names.md#protected-builtin-names).

### Cross-type equality

Applying `==` or `!=` to operands of differing source types triggers undefined behavior. No diagnostic, coercion, or cross-type `PartialEq` implementation is required. This includes shared versus mutable references, different array types, different container element types, and `Vec` versus array comparisons.

The [equality rules](builtin-traits.md#partialeq) define operand types, capability requirements, and same-type comparison. Lifetime arguments alone do not distinguish source types under [type identity](types.md#supported-types).

### Use compatibility

Tests may contain [use declarations](items/use-declarations.md), such as
`use rx::core::*;`, to compile the same source with Rust and the course support
crate. Their import paths and combinations are valid in that Rust environment.
Any imported name used by the program denotes an Rx builtin under its existing
spelling; unused imports and aliases are allowed. Tests never rely on a use
declaration to introduce another type, value, module, or usable alias into Rx.

Violations of these guarantees constitute undefined behavior and will not appear in
any positive, negative, or performance test. The compiler need not resolve imports
or diagnose missing imported items, conflicting imports, or other import validity errors.
Malformed use syntax must still be rejected by the parser with a standard syntax error.
After discarding use declarations, ordinary name, type, and place-mutability errors
elsewhere in the program remain compile errors.

### Lifetime validity

Lifetime declarations, annotations, arguments, bounds, and elision follow the supported Rust 2021 rules described in [References](references.md#lifetime-validity). Tests guarantee their validity throughout the program. Invalid lifetime use is undefined behavior and appears in no positive, negative, or performance test.

The compiler may discard this lifetime syntax after parsing and need not carry
it through semantic analysis or LLVM IR generation. The
[parser conventions](grammar.md#syntax-that-may-be-discarded-after-parsing)
explain which information remains necessary.

<details>
<summary>Excluded lifetime errors</summary>

Tests exclude undeclared or duplicate lifetimes, invalid shadowing, incorrect argument counts or placement, forbidden explicit late-bound arguments, ambiguous missing annotations, unsatisfied outlives bounds, invalid `'static` claims, rejected unused parameters, and bodies or uses that fail to uphold their annotations.

Malformed tokens and syntax retain their ordinary lexer and parser behavior. Other type and place-mutability errors remain compile errors. No lifetime or ownership checker is required; actual references must still obey the [borrowing rules](references.md#borrowing-rules).

</details>

### Zero-sized data

Unit `()` is valid as a function or control-flow result.

<details>
<summary>Excluded zero-sized uses</summary>

All other uses requiring observable zero-sized storage are undefined behavior, including:

- Zero-sized parameters, bindings, constants, statics, or assignment destinations.
- Empty structs or zero-sized fields.
- Zero-length arrays or zero-sized array elements.
- References to zero-sized values and dereferences of zero-sized data.

</details>

An empty `Vec<T>` with nonzero-sized `T` is valid and is not itself zero-sized. Its representation is implementation-defined.

Resource limits, stack size, allocation policy, and output constraints are part of the [backend contract](backend.md#resource-guarantees).
