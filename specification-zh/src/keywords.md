r[lex.keywords]
# 关键字

下列严格关键字和保留关键字不能在 Rx 的任何位置用作标识符。这些列表是[记法](notation.md#string-table-productions)中所述的字符串表产生式。

r[lex.keywords.strict]
<a id="strict-keywords"></a>
## 严格关键字

`as`, `break`, `const`, `continue`, `crate`, `else`, `enum`, `extern`, `false`, `fn`, `for`, `if`, `impl`, `in`, `let`, `loop`, `match`, `mod`, `move`, `mut`, `pub`, `ref`, `return`, `self`, `Self`, `static`, `struct`, `super`, `trait`, `true`, `type`, `unsafe`, `use`, `where`, `while`, `async`, `await`, `dyn`.

r[lex.keywords.reserved]
<a id="reserved-keywords"></a>
## 保留关键字

`abstract`, `become`, `box`, `do`, `final`, `macro`, `override`, `priv`, `typeof`, `unsized`, `virtual`, `yield`, `try`.

## 上下文名称

Rust 中的上下文关键字（例如 `union`、`macro_rules` 和 `gen`）可以在 Rx 中用作普通标识符。内建类型名、Trait 名以及标准 I/O 函数名受[受保护命名空间规则](names.md#protected-builtin-names)约束。
