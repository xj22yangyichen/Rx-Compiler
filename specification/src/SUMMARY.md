# Rx Compiler Specification

[Introduction](introduction.md)

- [Language scope and test guarantees](undefined-behavior.md)

# Source language

- [Notation](notation.md)
- [Lexical structure](lexical-structure.md)
    - [Input format](input-format.md)
    - [Keywords](keywords.md)
    - [Identifiers](identifiers.md)
    - [Comments](comments.md)
    - [Whitespace](whitespace.md)
    - [Tokens](tokens.md)
- [Parser conventions](grammar.md)
- [Crates and source files](crates-and-source-files.md)
- [Names and scopes](names.md)
    - [Paths](paths.md)
- [Items](items.md)
    - [Use declarations](items/use-declarations.md)
    - [Functions](items/functions.md)
    - [Structs](items/structs.md)
    - [Constant items](items/constant-items.md)
    - [Implementations](items/implementations.md)
    - [Associated items](items/associated-items.md)
    - [Lifetime parameters and bounds](items/generics.md)
- [Traits and attributes](traits-and-attributes.md)
    - [Builtin traits](builtin-traits.md)
- [Statements and bindings](statements.md)
- [Types and inference](types.md)
    - [Array types](types/array.md)
    - [Reference types](types/pointer.md)
    - [Box and Vec types](types/heap.md)
    - [Unit type](types/unit.md)
    - [Never type](types/never.md)
- [Expressions and evaluation](expressions.md)
    - [Literal expressions](expressions/literal-expr.md)
    - [Path expressions](expressions/path-expr.md)
    - [Block expressions](expressions/block-expr.md)
    - [Operator expressions](expressions/operator-expr.md)
    - [Grouped expressions](expressions/grouped-expr.md)
    - [Unit expressions](expressions/tuple-expr.md)
    - [Array and index expressions](expressions/array-expr.md)
    - [Struct expressions](expressions/struct-expr.md)
    - [Call expressions](expressions/call-expr.md)
    - [Method-call expressions](expressions/method-call-expr.md)
    - [Field access expressions](expressions/field-expr.md)
    - [Loop expressions](expressions/loop-expr.md)
    - [If expressions](expressions/if-expr.md)
    - [Return expressions](expressions/return-expr.md)
- [Constant contexts](const_eval.md)

# Values and execution

- [References and storage duration](references.md)
- [Box and Vec](heap.md)
- [Builtin I/O and program entry](undefined-behavior/builtin.md)
- [Backend](backend.md)

# Appendices

- [Grammar summary](grammar-summary.md)
<!-- - [Publication validation](open-decisions.md) -->
