<a id="return-expressions"></a>
# 返回表达式

r[expr.return.syntax]
```grammar,expressions
ReturnExpression -> `return` Expression?
```

`return expression` 对其操作数求值，并使用复制/移动语义从当前函数返回该值。`return;` 返回单元值 `()`。当执行到达函数体末尾且未提前返回时，函数尾表达式提供返回值。在所有情况下，返回值都必须与函数声明的返回类型兼容。

```rust,ignore
fn absolute(x: i32) -> i32 {
    if x < 0 {
        return -x;              // 显式提前返回
    }
    x                           // 通过尾表达式返回
}
```

类型确定与可达性参见 [Never 类型](../types/never.md)。
