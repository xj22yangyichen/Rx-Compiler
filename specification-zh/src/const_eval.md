# 常量上下文

常量上下文包括常量条目初始化器、数组类型长度和数组重复长度。每种上下文都使用下面定义的受限形式，包括对常量条目的引用。

## 允许的形式

```grammar,constants
ConstValue -> INTEGER_LITERAL | `true` | `false` | ConstantPath | `-` Magnitude | `(` ConstValue `)`

Magnitude -> INTEGER_LITERAL | ConstantPath | `(` Magnitude `)`

ConstantPath -> PathInExpression
```

`ConstantPath` 必须解析为常量条目，可以直接解析，也可以通过 `LIMIT`、`Config::LIMIT` 或 `Self::LIMIT` 这样的关联常量路径解析。指向局部变量、函数或其他值的路径属于静态形式错误。在此语法中，`Magnitude` 表示不带前导减号的整数字面量或常量路径，而不是无符号目标类型。负数形式要求有符号整数类型。这些形式允许 `123`、`LIMIT`、`-LIMIT`、`(-1)`、`-(1)`、`((true))` 以及有效的后缀和进制。

每个常量上下文包含整数或布尔字面量、常量条目引用、可选加括号的负整数或常量，或者包围另一个允许形式的括号。任何其他表达式都属于静态形式错误。

```rust,ignore
const COUNT: usize = 64;
const COPY: usize = COUNT;
const SENTINEL: i32 = (-1);
const ENABLED: bool = (true);

fn main() {
    let data: [i32; COUNT] = [0; COUNT];
    let n: usize = COPY; // ordinary expressions may also read a constant
}
```

常量路径会递归地通过其引用的常量条目求值。常量条目可以在使用它的条目之前或之后声明。依赖图必须无环：自引用或 `A -> B -> A` 这样的间接循环属于编译时错误。编译器必须在尝试计算相关常量之前检测循环。

<a id="typing-and-range"></a>
## 类型与范围

常量路径具有其目标条目声明的类型和值。常量条目初始化器具有声明的期望类型；数组长度是常量值，其期望类型为 `usize`。

字面量类型和有符号最小值情况遵循[整数字面量规则](expressions/literal-expr.md#integer-typing-and-range)。超出范围的字面量根据[整数字面量范围保证](undefined-behavior.md#integer-literal-range)属于未定义行为。普通类型错误和受限形式错误仍是编译时错误。

这里同样适用[传播排除规则](types.md#coercion-sites-and-expected-types)：`const N: isize = -1;` 属于未定义行为，因为它需要期望类型穿过一元减号。应改写为 `-1isize`。`const N: i32 = -1;` 有效，因为字面量 `-1` 默认使用 `i32`。

`[expr; 4]` 中的元素表达式是遵循[数组重复](expressions/array-expr.md#array-expressions)规则的普通表达式。只有其长度属于常量上下文。

对普通运行时表达式进行常量传播和折叠仍属于允许的优化。它们不会将这些表达式变成常量上下文，也不会改变其算术语义。
