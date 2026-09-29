r[lex.keywords]
# Keywords

The strict and reserved keywords listed below cannot be used as identifiers anywhere in Rx. The lists are string-table productions as described in [Notation](notation.md#string-table-productions).

r[lex.keywords.strict]
## Strict keywords

`as`, `break`, `const`, `continue`, `crate`, `else`, `enum`, `extern`, `false`, `fn`, `for`, `if`, `impl`, `in`, `let`, `loop`, `match`, `mod`, `move`, `mut`, `pub`, `ref`, `return`, `self`, `Self`, `static`, `struct`, `super`, `trait`, `true`, `type`, `unsafe`, `use`, `where`, `while`, `async`, `await`, `dyn`.

r[lex.keywords.reserved]
## Reserved keywords

`abstract`, `become`, `box`, `do`, `final`, `macro`, `override`, `priv`, `typeof`, `unsized`, `virtual`, `yield`, `try`.

## Contextual names

Contextual keywords in Rust (such as `union`, `macro_rules`, and `gen`) may be used as ordinary identifiers in Rx. Built-in type and trait names and standard I/O function names are governed by the [protected namespace rules](names.md#protected-builtin-names).
