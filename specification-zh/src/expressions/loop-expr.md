<a id="loop-expressions"></a>
# 循环表达式

<a id="while-and-loop"></a>
## While 与 loop

```grammar,expressions
LoopExpression -> InfiniteLoopExpression | PredicateLoopExpression

InfiniteLoopExpression -> `loop` BlockExpression

PredicateLoopExpression -> `while` Conditions BlockExpression

BreakExpression -> `break` Expression?

ContinueExpression -> `continue`
```

`while condition { ... }` 在每次迭代前重新对 `bool` 条件求值，其求值结果为单元值 `()`。条件周围的括号是可选的。`loop { ... }` 重复执行，直到控制流离开循环；`break` 的值决定循环表达式的结果类型。这两种形式的主体块都必须在类型上与单元类型 `()` 兼容；发散的主体也兼容。主体块中非单元类型的尾表达式不会成为循环表达式的值。因此，`while false { 1 }` 和 `loop { 1 }` 会导致编译时类型错误，而 `loop` 表达式可以通过 `break value;` 产生值。

`break;` 以单元值退出最近的循环。携带值的 break 要求目标是 `loop`。循环的预期类型通过[一对一强制转换](../types.md#coercion-sites-and-expected-types)作用于其中所有 break 的值；没有预期类型时，由 [LUB 强制转换](../types.md#least-upper-bound-coercions)确定它们的公共类型。`continue;` 开始最近的外围循环的下一次迭代（包括重新对 `while` 条件求值）。在循环外使用 `break` 或 `continue` 会导致编译错误。

出现在 while 条件中的 break 或 continue 必须以嵌套在该条件内的循环为目标。以包含该条件的 while 或任何外层循环为目标都会导致编译错误。例如，允许 `while loop { break false; } {}`。目标由语法上的循环嵌套关系确定。

```rust,ignore
fn until(limit: i32) -> i32 {
    let mut n = 0;
    loop {
        if n == limit { break n; }
        n += 1;
    }
}
```

关于 `loop` 表达式何时具有 never 类型，参见 [Never 类型](../types/never.md#loop-expressions)；关于控制流离开块之后仍然适用的检查，参见[不可达代码](../types/never.md#unreachable-code)。
