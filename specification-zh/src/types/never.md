<a id="never-type"></a>
# Never 类型

never 类型通常写作 `!`，描述不会正常完成的表达式。它仅在内部使用；`!` 不是有效的源代码类型标注。

`return`、`break` 和 `continue` 表达式具有 never 类型：它们转移控制流，而非在出现位置产生值。所有执行路径都通过这些转移提前退出的[块表达式](../expressions/block-expr.md)同样具有 never 类型。

在受支持的[强制转换上下文](../types.md#conversions-and-coercions)中，never 类型可以强制转换为任意期望结果类型。没有期望类型时，never 结果不约束其他结果的公共类型；如果所有结果都是 never，则公共类型为 never。这允许一个 if 分支提前返回，而另一个分支产生值，包括引用或容器。单元类型 `()` 和 never 类型 `!` 不同：求值得到 `()` 的表达式会正常完成，而 never 类型的表达式不会。

```rust,ignore
fn choose_or_return(flag: bool) -> i32 {
    let n: i32 = if flag { return 7; } else { 3 };
    n + 1
}
```

发散的函数体满足任意声明的返回类型，例如 `fn forever() -> i32 { loop {} }`。对该函数的调用具有其声明的类型 `i32`。never 行为在各函数体内局部确定。

<a id="loop-expressions"></a>
## 循环表达式

如果没有以某个 `loop` 表达式为目标的 `break` 表达式，则该 `loop` 具有 never 类型。嵌套循环中的 `break` 以该嵌套循环为目标，因此 `loop { loop { break; } }` 仍具有 never 类型。

如果有任何 `break` 表达式以该循环为目标，它们的值遵循[公共结果规则](../types.md#least-upper-bound-coercions)，在给出循环的期望类型时使用该类型；`break;` 提供 `()`。如果每个 break 操作数都具有 never 类型，则循环也具有 never 类型。即使不可达，break 表达式也参与类型推断：`loop { if false { break 1i32; } }` 的类型为 `i32`。

这些规则适用于 `loop`。`while` 表达式的类型为 `()`，即使其条件恒为真。

<a id="unreachable-code"></a>
## 不可达代码

不可达代码仍需要普通的语义检查，包括名称解析、类型检查和位置可变性检查。被明确归类为未定义行为的情况不要求诊断。

对于测试保证范围内的程序，生成的代码必须遵守控制流转移。`return` 退出当前函数，`break` 退出其目标循环，`continue` 进入下一次迭代。被这些转移绕过的代码不得执行。

根据[测试保证](../undefined-behavior.md#test-guarantees)，不可达代码中的位置可变性违规具有未定义行为。这包括对不可变位置的普通赋值和复合赋值、对不可变位置的可变借用，以及需要无法提供的可变访问的方法或索引调整。该规则涵盖局部变量、结构体字段、索引元素和解引用位置。不要求诊断这些违规。例如：

```rust,ignore
fn example() {
    let value = 1;
    let shared = &value;
    return;
    value = 2; // 未定义行为：return 之后对不可变局部变量赋值
    let borrowed = &mut *shared; // 未定义行为：return 之后进行可变借用
    *shared = 3; // 未定义行为：return 之后通过共享引用写入
}
```

这一排除专门针对位置可变性。名称解析错误、类型不匹配错误，以及赋值目标不是位置表达式的情况，即使在不可达代码中也仍然是必须报告的编译错误。

编译器实现不必在 AST 或 IR 中显式表示 never 类型，也不必解决停机问题或证明终止性。
