<a id="if-expressions"></a>
# If 表达式

r[expr.if.syntax]
```grammar,expressions
IfExpression ->
    `if` Conditions BlockExpression
    (`else` (BlockExpression | IfExpression))?

Conditions -> Expression _except an unparenthesized [StructExpression] at the condition/body boundary_
```

```rust,ignore
fn choose(flag: bool) -> i32 {
    if flag { 1 } else { 2 }
}
```

条件表达式必须求值为 `bool` 类型；外围括号是可选的。支持 `else if` 链；执行时按顺序对条件求值，并执行第一个求值为 `true` 的条件对应的主体（如果没有条件匹配且存在 `else` 块，则执行该块）。处于条件/主体边界的结构体构造需要如下所述的显式界定。

当条件/主体边界处未加括号的 `Name {` 可能开始一个结构体字面量时，将 `{` 视为条件成立时执行的块的起始。要将这种结构体构造放入条件中，需显式界定它，例如 `if (S { flag: true }).flag { ... }`。函数实参（如 `if check(S { flag: true }) { ... }`）已经受到界定。这一规则并不意味着 if 后的每个花括号都开始其条件成立时执行的块：`if { true } { ... }` 的条件是块表达式，后面还有一个单独的条件成立时执行的块。

存在 `else` 时，外围的预期类型通过[一对一强制转换](../types.md#coercion-sites-and-expected-types)作用于两个结果分支。没有预期类型时，由 [LUB 强制转换](../types.md#least-upper-bound-coercions)确定它们的公共类型。具有 never 类型 `!` 的分支总能强制转换为另一分支的类型。没有 `else` 时，表达式具有单元类型 `()`，且 `then` 块的预期类型为 `()`。
