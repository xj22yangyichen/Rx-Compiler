<a id="block-expressions"></a>
# 块表达式

r[expr.block.syntax]
```grammar,expressions
BlockExpression -> `{` Statements? `}`

Statements ->
      Statement+
    | Statement+ ExpressionWithoutBlock
    | ExpressionWithoutBlock
```

块包含[语句](../statements.md)和一个可选的末尾表达式，并用花括号包围。语句按照源代码顺序执行。

块的类型和结果值按以下方式确定：

- 如果块有尾表达式（末尾不带分号的表达式），则该表达式决定块的类型，并在执行到它时决定块的结果值。即使之前的语句发散，尾表达式仍决定块的类型（不可达的尾表达式仍会根据任何期望类型进行类型检查）。
- 如果块没有尾表达式且正常完成，则其类型为 `()`，求值结果为 `()`。
- 如果块没有尾表达式且不会正常完成，也就是说每条执行路径都通过控制流转移（`return`、`break` 或 `continue`）提前退出，或通过求值发散表达式退出，则块具有 [never 类型](../types/never.md) `!`。如果某些路径提前退出，而另一条路径到达末尾且没有尾表达式，则块以 `()` 类型正常完成。

```rust,ignore
fn select(flag: bool) -> i32 {
    let base = if flag { 10 } else { 20 };
    { base + 1 } // 内部块和函数体都产生 i32 值
}
```

即使形式语法允许在语句序列中解析不带分号的 [ExpressionWithBlock](../expressions.md)，当它位于块的最终位置时，也会作为该块的尾表达式，并决定块的值。

块和末尾语句的解析遵循[语句边界规则](../statements.md#statement-boundary)；局部变量的存储遵循[存储期](../references.md#storage-and-implementation)规则。
