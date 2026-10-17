<a id="operators"></a>
# 运算符

```grammar,expressions
OperatorExpression ->
      BorrowExpression
    | DereferenceExpression
    | NegationExpression
    | ArithmeticOrLogicalExpression
    | ComparisonExpression
    | LazyBooleanExpression
    | TypeCastExpression
    | AssignmentExpression
    | CompoundAssignmentExpression
```

<a id="arithmetic-and-bits"></a>
## 算术与位运算

```grammar,expressions
NegationExpression ->
      `-` Expression
    | `!` Expression

ArithmeticOrLogicalExpression ->
      Expression `+` Expression
    | Expression `-` Expression
    | Expression `*` Expression
    | Expression `/` Expression
    | Expression `%` Expression
    | Expression `&` Expression
    | Expression `|` Expression
    | Expression `^` Expression
    | Expression `<<` Expression
    | Expression `>>` Expression
```

算术运算符 `+`、`-`、`*`、`/` 和 `%` 作用于类型兼容的整数操作数，不进行隐式整数转换。一元负号要求有符号整数。按位运算符 `&`、`|`、`^` 和 `!` 作用于整数；它们也提供非短路的布尔运算。布尔 `!` 对 `bool` 操作数执行逻辑非运算。

原始类型的算术与位运算支持下表中的标量和引用操作数。T 表示一种受支持的标量类型，I 表示一种受支持的有符号整数类型，L/R 表示各自独立选择的受支持整数类型。

| 运算 | 操作数类型 | 结果 |
| --- | --- | --- |
| 二元算术 `+`、`-`、`*`、`/`、`%` | 每个操作数为 T 或 &T，两侧的整数类型 T 相同 | T |
| 二元按位运算 `&`、<code>&#124;</code>、`^` | 每个操作数为 T 或 &T，两侧的整数或 bool 类型 T 相同 | T |
| 移位 `<<`、`>>` | 左侧为 L 或 &L；右侧为 R 或 &R | L |
| 一元 `-` | I 或 &I | I |
| 一元 `!` | T 或 &T，其中 T 为整数或 bool 类型 | T |
| 算术/按位复合赋值 | 原始类型 T 的可变位置；右侧为对应运算的 T 或 &T | 单元值 |
| 移位复合赋值 | 原始整数类型 L 的可变位置；右侧为 R 或 &R | 单元值 |

预期类型不会通过二元或一元运算符传播到其操作数。需要这种传播来推断整数类型的程序会导致[未定义行为](../types.md#coercion-sites-and-expected-types)。例如，`let n: u32 = 1 + 2;` 和 `let n: isize = -1;` 都属于未定义行为；写成 `1u32 + 2u32` 和 `-1isize` 则显式提供了类型。

复合赋值同样不会根据目标推断右操作数的整数类型。对于 `n: u32`，`n += 1;` 需要这种推断，因而属于未定义行为；当 `n` 可变时，`n += 1u32;` 是有效的。表中的所有操作数要求仍然适用。

<details>
<summary>引用操作数的细节</summary>

这些算术/按位运算的引用变体接受一层共享引用，而不支持任意的自动解引用。例如，对于兼容的原始类型，支持 `&a + &b`、`-&a` 和 `x += &y`；不支持 `&&a + &b`、`-&mut a` 和 `x += &mut y`。可以显式解引用可变引用操作数来取得标量值。值为引用的赋值目标不会被自动解引用：应使用 `*p += rhs` 来更新 p 所指向的原始类型值。

这些运算计算得到标量结果；不会仅因为操作数是引用就修改被引用的值。对引用操作数求值会产生其引用值；运算所需的标量加载在各操作数求值之后、执行该运算时发生。复合赋值的顺序在下文规定。

该表定义了这些运算的全部内建操作数组合。

</details>

所有受支持的整数都是 32 位。运行时的加法、减法、乘法以及有符号取负使用二进制补码算术进行回绕，与禁用溢出检查的 Rust 一致。优化和未优化的代码具有相同的行为。除非编译器已经证明特定运算不会溢出，否则 LLVM 溢出标志不得断言该运算不会溢出。

有符号除法向零截断；非零余数的符号与被除数相同。除数为零的除法或取余，以及有符号最小整数（`-2147483648i32`）除以 -1 或对 -1 取余，均被排除在有效执行之外。不要求进行检查或提供 panic 运行时。

移位接受类型可以不同的整数操作数。结果具有左操作数的类型。在运行时，移位量通过掩码只保留最低 5 位。有符号右移是算术右移；无符号右移是逻辑右移。该掩码规则也适用于经过常量折叠的普通表达式。

<a id="comparison-and-logic"></a>
## 比较与逻辑

```grammar,expressions
ComparisonExpression ->
      Expression `==` Expression
    | Expression `!=` Expression
    | Expression `>` Expression
    | Expression `<` Expression
    | Expression `>=` Expression
    | Expression `<=` Expression

LazyBooleanExpression ->
      Expression `||` Expression
    | Expression `&&` Expression
```

相等性运算符使用 [PartialEq 与相等性规则](../builtin-traits.md#partialeq)，包括操作数类型确定、隐式借用和结构化比较。不同源类型之间的相等性比较属于未定义行为；参见[跨类型相等性保证](../undefined-behavior.md#cross-type-equality)。

标量顺序比较使用 `<`、`<=`、`>`、`>=`，作用于类型匹配的整数和 bool（false 排在 true 之前）。引用操作数比较目标值，而不是地址。其引用层在每个深度都必须具有匹配的可变性，并最终指向同一种受支持的标量类型。还支持一种额外的操作数组合：类型为 `&T` 的左操作数允许类型为 `&mut T` 的右操作数，后者会调整为 `&T`，且被引用类型 `T` 必须完全相同。这种特定于运算符的调整不依赖预期结果类型，也不会改写内层引用。

例如，对于匹配且支持顺序比较的标量，`&a < &b`、`&mut a < &mut b` 和 `&a < &mut b` 都有效；`&mut a < &b` 和 `&&a < &&mut b` 则无效。既不支持值与引用之间的自动比较（如 `a < &b`），也不会为了顺序比较而自动解引用 `Box`。

`&&` 和 `||` 要求 bool 值，返回 bool，并从左到右进行短路求值。它们没有引用操作数变体：`&true && true` 是无效的。整数不能被当作真值使用。不直接允许 `a < b < c` 这样的链式比较；必须使用显式括号且中间类型兼容（例如 `(a < b) < c`，其中 `a < b` 求值为 `bool`），或使用逻辑合取（`a < b && b < c`）。

<a id="casts"></a>
## 显式类型转换

```grammar,expressions
TypeCastExpression -> Expression `as` TypeNoBounds
```

`as` 支持整数到整数以及 bool 到整数的显式转换。由于所有整数都是 32 位，整数转换会保留 32 位的位模式，并按照目标类型的有符号性解释它。False 变为 0，true 变为 1。不支持整数到 bool 或引用到整数的转换。转换的目标类型不会向操作数提供预期类型：在 `1 as u32` 中，字面量是 `i32`，转换产生 `u32`。

<a id="cast-parsing"></a>
### 显式类型转换的解析

<details>
<summary>区分泛型实参与运算符</summary>

[运算符优先级表](../expressions.md#precedence)控制表达式的嵌套。在显式类型转换的类型路径段之后，`<` 开始 [GenericArgs]，而不是比较。`<<` 的前导 `<` 同样会进入类型实参解析。为转换加上括号可以明确所需的运算：`(x as usize) < y` 和 `(x as usize) << y`。带括号的类型已经闭合了类型语法，因此 `x as (usize) < y` 和 `x as (usize) << y` 也会解析为比较和移位。`<=`、`>`、`>>` 和 `==` 等运算符在转换类型之后遵循普通优先级。

解析器检查得到的语法；名称解析检查其类型实参。因此，`x as usize<i32>` 具有类型路径语法树，但由于原始类型 usize 不接受类型实参，会导致编译错误。

</details>

<a id="borrow-dereference-and-assignment"></a>
## 借用、解引用与赋值

```grammar,expressions
BorrowExpression ->
      (`&` | `&&`) Expression
    | (`&` | `&&`) `mut` Expression

DereferenceExpression -> `*` Expression

AssignmentExpression -> Expression `=` Expression

CompoundAssignmentExpression ->
      Expression `+=` Expression
    | Expression `-=` Expression
    | Expression `*=` Expression
    | Expression `/=` Expression
    | Expression `%=` Expression
    | Expression `&=` Expression
    | Expression `|=` Expression
    | Expression `^=` Expression
    | Expression `<<=` Expression
    | Expression `>>=` Expression
```

`&place` 形成共享引用；`&mut place` 要求可变位置。借用作用于值表达式时会物化一个临时值。`*reference` 访问其目标，`*box` 根据 [Box 规则](../heap.md#box-access-and-moves)访问其拥有的 `T`。包括 `Vec` 在内的其他类型不能作为解引用操作数。在前缀借用位置，`&&x` 表示 `&(&x)`；中缀 `left && right` 则是短路布尔与。

每个显式 `*` 执行一次解引用。解引用 `&T` 提供共享访问；解引用 `&mut T` 提供可变访问，除非它是通过共享引用到达的。存储在不可变局部变量、结构体、数组或 `Box` 中的 `&mut T` 仍然可以通过解引用修改其目标。一旦位置的访问路径经过共享引用，后续解引用就无法恢复可变访问。

下面的所有参数都是不可变绑定。每个函数仍然可以通过其存储的可变引用修改整数：

```rust,ignore
struct Holder<'a> {
    value: &'a mut i32,
}

fn through_local(value: &mut i32) {
    *value = 2; // 一次解引用即可到达整数
}

fn through_struct(holder: Holder<'_>) {
    *holder.value = 2; // 修改整数，而不是持有引用的字段
}

fn through_array(values: [&mut i32; 1]) {
    *values[0] = 2; // 先通过索引获取引用，再解引用
}

fn through_box(value: Box<&mut i32>) {
    **value = 2; // 第一个 * 访问 Box 内容；第二个 * 沿 &mut i32 解引用
}
```

以下函数无法编译，因为其第一次解引用经过共享引用，使内层可变引用无法授予可变访问权限：

```rust,ignore
fn through_shared_reference(value: &&mut i32) {
    **value = 2; // 编译错误：外层 & 只提供共享访问
}

fn through_shared_box(value: &Box<&mut i32>) {
    ***value = 2; // 编译错误：依次经过 &、Box、&mut，仍然经过了共享访问
}
```

`place = value` 按照所要求的复制/移动行为存储值。复合赋值将对应运算应用于目标处当前的标量值。两种形式都产生单元值。即使引用运算的降低最终使用相同的标量指令，下列顺序规则也仍然适用。

<a id="assignment-destinations"></a>
## 赋值目标

赋值目标是一个可变[位置](../expressions.md#places-and-values)。被赋的值必须具有兼容类型。不满足这些要求的目标会导致编译错误。

`s = other;` 和 `a = other_array;` 这样的赋值会通过单次操作将整个结构体或数组值赋给目标位置。

<a id="evaluation-order"></a>
## 求值顺序

| 表达式形式 | 求值顺序 |
| --- | --- |
| 一元标量或共享引用运算 | 先对操作数求值，再执行运算 |
| 普通二元算术、按位、移位或比较运算，包括引用变体 | 先对左操作数求值，再对右操作数求值，然后执行运算 |
| 惰性布尔 `&&` / <code>&#124;&#124;</code> | 先对左侧 bool 求值；仅在需要时对右侧求值 |
| 普通赋值 `place = value` | 先对右侧值求值，再计算目标地址，然后存储 |
| 右操作数为原始类型标量或共享引用的复合赋值 | 先对右操作数求值，再计算目标地址，然后执行标量读取/更新 |

所有受支持的复合赋值都使用相同的右侧优先顺序，与右操作数是标量还是共享引用无关。每个操作数表达式和目标地址计算都只求值一次。目标的旧值在操作数求值之后读取。引用类型的右操作数先求值得到其引用值；在执行标量运算时才读取其目标。借用有效性仍由测试保证，不要求借用检查器进行检查。

<details>
<summary>复合赋值顺序示例</summary>

根据课程规则：

```rust,ignore
fn destination(p: &mut i32) -> &mut i32 {
    println_i32(2);
    p
}

fn right_value() -> i32 {
    println_i32(1);
    3
}

fn right_reference(p: &i32) -> &i32 {
    println_i32(1);
    p
}

fn main() {
    let mut x = 0;
    let y = 3;
    *destination(&mut x) += right_value();       // 先输出 1，再输出 2
    *destination(&mut x) += right_reference(&y); // 先输出 1，再输出 2
}
```

官方测试不依赖复合赋值两个操作数的相对求值顺序。上述示例展示了规定的行为，但评分测试套件不检查操作数之间的求值顺序。

</details>

覆盖或丢弃已有值不会清除其在内存中的字节，也不会将这些字节置零。替换容器不要求递归析构或立即释放内存；根据[堆清理规则](../heap.md#heap-cleanup)，其旧的堆存储可以继续保留。
