<a id="literal-expressions"></a>
# 字面量表达式

r[expr.literal.syntax]
```grammar,expressions
LiteralExpression -> INTEGER_LITERAL | `true` | `false`
```

字面量表达式产生整数值或布尔值。[INTEGER_LITERAL] 定义整数词法单元的书写形式。

注意，单元值 `()` 使用的是 [UnitExpression]。

<a id="integer-typing-and-range"></a>
## 整数类型确定与范围

整数后缀固定其类型。没有后缀时，如果提供了预期整数类型，就使用该类型；否则使用 `i32`。需要通过二元运算符、一元运算符或共享/可变借用传播类型信息的推断属于[未定义行为](../types.md#coercion-sites-and-expected-types)。根据后续操作数或后面的使用情况来选择字面量的类型，同样会导致[未定义行为](../types.md#coercion-sites-and-expected-types)。运行时整数位宽在[类型](../types.md)中规定。

前导负号构成 [NegationExpression]。一元负号作用于字面量时允许表示有符号最小值：`-2147483648i32` 使用大小为 2147483648 的整数词法单元，表示有效的 i32 最小值。带括号的负数字面量（如 `-(2147483648i32)`）遵循相同规则。

通过后缀、预期类型或 `i32` 回退规则选择类型后，如果整数字面量的值超出该类型可表示的范围，则会导致未定义行为；参见[整数字面量范围保证](../undefined-behavior.md#integer-literal-range)。

```rust,ignore
let a = 42;             // 默认为 i32
let b = 42u32;          // 通过显式后缀确定为 u32
let c: usize = 42;      // 由预期类型确定为 usize
let min = -2147483648i32; // 有效的 i32 最小值
```
