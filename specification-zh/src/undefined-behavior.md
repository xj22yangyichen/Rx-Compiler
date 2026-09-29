<a id="language-scope-and-test-guarantees"></a>
# 语言范围与测试保证

<a id="definition"></a>
## 定义

我们区分有效程序、编译错误和未定义行为：

| 类别 | 对编译器的要求 | 测试政策 |
| --- | --- | --- |
| 有效程序 | 编译并产生指定行为 | 可出现在任何适用的测试中 |
| 编译错误 | 正常拒绝；诊断措辞未指定 | 可出现在反例测试中 |
| 未定义行为 | 不要求任何诊断或行为 | 按其定义规则中的说明排除 |

即使所提供的解析器能识别某种不受支持的语法，也可以在语言子集边界拒绝它。除非某种源代码形式被明确归类为未定义行为，否则普通的名称、类型和位置可变性错误仍是编译错误，包括不可达代码中的此类错误。编译器不必证明终止性、借用有效性或所有权有效性。对于被排除的执行，不要求运行时检查、panic 处理和栈展开。

<a id="language-subset"></a>
## 语言子集

Rx 是 Rust 2021 的子集。每个 Rx 源程序都使用有效的 Rust 语法，但本规范定义自己的内建环境和所要求的运行时行为。

Rx 包含：

- 函数、具名字段结构体、常量和固有实现。
- 用于 Rust 兼容性的顶层 [use 声明](items/use-declarations.md)，在解析后忽略。
- 标识符绑定和局部类型推断。
- 整数、布尔值、定长数组、引用以及内建 `Box` 和 `Vec` 容器。
- `if`、`while`、`loop`、`return`、`break` 和 `continue` 控制流。
- 结构体的 `Copy`、`Clone`、`PartialEq` 和 `Eq` 派生。

Rx 子集不包含：

- 用户 Trait、Trait 约束和 Trait 对象。
- 枚举、非单元元组、元组结构体、元组变体和单元变体。
- 模式匹配、解构和守卫。
- 类型参数和一般泛型。支持生命周期语法，但其有效性由保证提供，而非检查。
- 模块声明和导入解析、可见性、隐私性、宏以及任意属性。仅按 [use 兼容性保证](#use-compatibility)所述接受 use 声明。
- 不安全 Rust，以及此处定义的内建项之外的标准库。
- 常量泛型和受限[常量上下文](const_eval.md)之外的一般常量求值。常量传播仍是可选优化。
- 整数溢出检查。

[类型](types.md)、[表达式](expressions.md)和[语法汇总](grammar-summary.md)定义完整子集。该语法之外的形式不受支持。

<a id="test-guarantees"></a>
## 测试保证

以下源代码形式触发未定义行为，绝不会出现在测试套件中，包括在适用时出现在不可达代码或常量上下文中的情况：

| 未定义行为 | 定义规则 |
| --- | --- |
| 整数字面量超出其确定的类型范围 | [整数字面量范围](#integer-literal-range) |
| 整数推断需要通过二元运算、一元运算或共享/可变借用传播期望类型 | [期望类型传播](types.md#coercion-sites-and-expected-types) |
| 在 `Box::new(...)` 或 `Vec::new()` 构造函数调用中省略显式的具体类型实参 | [内建构造函数](heap.md#builtin-signatures) |
| 将函数用作值 | [函数值](expressions/path-expr.md#function-values) |
| 声明名为 `u32`、`isize`、`usize`、`bool`、`Box`、`Vec`、`Copy`、`Clone`、`PartialEq` 或 `Eq` 的结构体 | [受保护的内建名称](names.md#protected-builtin-names) |
| 名为 `get_i32`、`print_i32` 或 `println_i32` 的局部绑定 | [受保护的内建名称](names.md#protected-builtin-names) |
| 不可达代码中的位置可变性违规，包括赋值、可变借用或可变接收者调整 | [不可达代码](types/never.md#unreachable-code) |
| 数组或 `Vec` 的索引操作数具有 never 类型 `!` | [索引操作数类型](expressions/array-expr.md#index-operand-types) |
| 文档注释（`///`、`//!`、`/**`、`/*!`） | [文档注释](comments.md#documentation-comments) |
| let 或参数名称与可见的非限定常量冲突 | [常量名称冲突](#constant-name-collisions) |
| 不同源类型之间的相等性比较 | [跨类型相等性](#cross-type-equality) |
| 两个类型都不能成为公共目标的 LUB 强制转换 | [最小上界强制转换](types.md#least-upper-bound-coercions) |
| 点调用中存在多个不同的匹配方法 | [方法查找](expressions/method-call-expr.md#method-lookup) |
| 违反 Rust 兼容性保证的 use 声明 | [use 兼容性](#use-compatibility) |
| 无效的生命周期声明、使用、约束或省略 | [生命周期有效性](#lifetime-validity) |
| 单元结果例外之外的可观察零大小数据使用 | [零大小数据](#zero-sized-data) |

这些保证约束有效的源程序。编译期间引入的任何无效内存访问、错误终止或错误程序输出仍然是编译器缺陷。

<a id="integer-literal-range"></a>
### 整数字面量范围

整数字面量的类型依次由其后缀、期望整数类型或 `i32` 回退类型选定。无后缀整数字面量在没有期望类型的情况下求值时，若其值超出 `i32` 范围，则构成未定义行为。

整数字面量溢出具有未定义行为。[字面量规则](expressions/literal-expr.md#integer-typing-and-range)仍允许 `-2147483648i32` 和 `-(2147483648i32)` 这样的有符号最小值。

<a id="constant-name-collisions"></a>
### 常量名称冲突

名称与可见的非限定常量相同的 `let` 绑定或函数参数触发未定义行为，与可变性或声明顺序无关。不要求诊断或将其解释为常量模式。只能通过 `Type::NAME` 访问的关联常量不会排除局部 `NAME`。与内建名称的冲突遵循[受保护的内建项规则](names.md#protected-builtin-names)。

<a id="cross-type-equality"></a>
### 跨类型相等性

对不同源类型的操作数应用 `==` 或 `!=` 会触发未定义行为。不要求诊断、强制转换或跨类型的 `PartialEq` 实现。这包括共享引用与可变引用之间、不同数组类型之间、不同容器元素类型之间，以及 `Vec` 与数组之间的比较。

[相等性规则](builtin-traits.md#partialeq)定义操作数类型、能力要求和同类型比较。根据[类型同一性](types.md#supported-types)，仅生命周期实参不同不足以区分源类型。

<a id="use-compatibility"></a>
### use 兼容性

测试可以包含 [use 声明](items/use-declarations.md)，例如 `use rx::core::*;`，以便使用 Rust 和课程支持 crate 编译同一份源代码。它们的导入路径和组合在该 Rust 环境中有效。程序使用的任何导入名称都以其现有拼写表示某个 Rx 内建项；允许未使用的导入和别名。测试绝不依赖 use 声明向 Rx 引入其他类型、值、模块或可用别名。

违反这些保证构成未定义行为，不会出现在任何正例、反例或性能测试中。编译器不必解析导入，也不必诊断缺失的导入项、冲突的导入或其他导入有效性错误。格式错误的 use 语法仍必须由解析器以标准语法错误拒绝。丢弃 use 声明后，程序其他位置的普通名称、类型和位置可变性错误仍然是编译错误。

<a id="lifetime-validity"></a>
### 生命周期有效性

生命周期声明、标注、实参、约束和省略遵循[引用](references.md#lifetime-validity)中描述的受支持 Rust 2021 规则。测试保证它们在整个程序中有效。无效的生命周期使用属于未定义行为，不会出现在任何正例、反例或性能测试中。

编译器可以在解析后丢弃这些生命周期语法，无需将其带入语义分析或 LLVM IR 生成。[解析器约定](grammar.md#syntax-that-may-be-discarded-after-parsing)说明哪些信息仍需保留。

<details>
<summary>被排除的生命周期错误</summary>

测试排除未声明或重复的生命周期、无效遮蔽、错误的实参数量或位置、禁止的显式后期绑定实参、有歧义的缺失标注、不满足的存活期约束、无效的 `'static` 声明、会被拒绝的未使用参数，以及未能遵守其标注的函数体或使用。

格式错误的词法单元和语法仍按普通的词法分析器和解析器行为处理。其他类型和位置可变性错误仍是编译错误。不要求生命周期或所有权检查器；实际引用仍必须遵守[借用规则](references.md#borrowing-rules)。

</details>

<a id="zero-sized-data"></a>
### 零大小数据

单元 `()` 作为函数或控制流结果是有效的。

<details>
<summary>被排除的零大小使用</summary>

所有其他需要可观察零大小存储的使用都属于未定义行为，包括：

- 零大小的参数、绑定、常量、静态项或赋值目标。
- 空结构体或零大小字段。
- 零长度数组或零大小数组元素。
- 指向零大小值的引用以及对零大小数据的解引用。

</details>

当 `T` 大小非零时，空 `Vec<T>` 有效，且它本身并非零大小。其表示由实现定义。

资源限制、栈大小、分配策略和输出约束属于[后端契约](backend.md#resource-guarantees)的一部分。
