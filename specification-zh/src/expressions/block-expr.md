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

- 如果块有尾表达式（末尾不带分号的表达式），则该表达式决定块的类型，并在执行到它时决定块的结果值。
- 如果块没有尾表达式且正常完成，则其类型为 `()`，求值结果为 `()`。
- 如果块内的每条执行路径都通过控制流转移（`return`、`break` 或 `continue`）提前退出，则块不会正常完成，其类型为 [never 类型](../types/never.md) `!`。如果某些路径提前退出，而另一条路径到达末尾且没有尾表达式，则块以 `()` 类型正常完成。

```rust,ignore
fn select(flag: bool) -> i32 {
    let base = if flag { 10 } else { 20 };
    { base + 1 } // 内部块和函数体都产生 i32 值
}
```

块和末尾语句的解析遵循[语句边界规则](../statements.md#statement-boundary)；局部变量的存储遵循[存储期](../references.md#storage-and-implementation)规则。
