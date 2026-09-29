# 解析器约定

语言语法由各章节中的 Lexer 和 Syntax 代码块定义。[语法汇总](grammar-summary.md)将这些产生式集中在一处。本章索引优先级和表达式边界规则，并规定词法单元的上下文解释方式。

<details>
<summary>按源代码形式划分的语法覆盖范围</summary>

每个 [Type] 或 [Expression] 的出现位置都允许其对应产生式定义的所有可选形式。这种组合可以递归应用：结构体字段可以包含 `Vec<Box<T>>` 这样的嵌套容器类型，构造函数实参可以包含块，构造函数结果也可以继续进行索引或方法调用。

| Source form | Defining productions | Examples |
| --- | --- | --- |
| Identifiers and integers | [IDENTIFIER], [INTEGER_LITERAL] | `_value`, `123_i32`, `0b1010` |
| Lifetime tokens and forms | [LIFETIME_TOKEN], [Lifetime] | `'a`, `'static`, `'_` |
| Lifetime parameters and bounds | [GenericParams], [LifetimeParam], [WhereClause] | `fn f<'a>(x: &'a i32)`, `where 'a: 'b` |
| Source compilation unit | [Crate], [Item] | Top-level use declarations, functions, structs, constants, impls |
| Use declarations | [UseDeclaration], [UseTree], [UsePath] | `use rx::core::*;`, `use rx::core::{get_i32, println_i32};` |
| Functions and receivers | [Function], [FunctionParameters], [ShorthandSelf] | `fn f(v: Vec<i32>) -> Box<i32> { ... }`, `&mut self` |
| Struct declarations and construction | [StructStruct], [StructExpression] | `struct S { x: i32 }`, `S { x: 1 }`, `Self { x: 1 }` |
| Associated items | [InherentImpl], [AssociatedItem] | `impl S { fn new() -> Self { ... } }` |
| Derive attributes | [OuterAttribute], [DeriveAttribute] | `#[derive(Clone, PartialEq, Eq)]` |
| Bindings and statements | [LetStatement], [ExpressionStatement] | `let mut v = Vec::<i32>::new();`, `v.push(1);` |
| Type names, arrays, references, and unit | [TypePath], [ArrayType], [ReferenceType], [UnitType] | `Self`, `[Vec<i32>; 2]`, `&mut Box<i32>`, `()` |
| Container types | [TypePath], [TypePathSegment], [GenericArgs] | `Box<[i32; 4]>`, `Vec<Box<Node>>`, `Vec<Vec<i32,>,>` |
| Container construction | [CallExpression], [PathInExpression] | `Box::<i32>::new(7)`, `Vec::<Box<i32>>::new()` |
| Paths, calls, fields, and indexing | [PathExpression], [CallExpression], [MethodCallExpression], [FieldExpression], [IndexExpression] | `S::new()`, `value.clone()`, `value.field`, `values[0]` |
| Arrays, grouping, and unit values | [ArrayExpression], [GroupedExpression], [UnitExpression] | `[1, 2, 3]`, `[0; 4]`, `(value)`, `()` |
| Operators | [OperatorExpression] | `*p`, `&mut values[0]`, `a + b`, `x += &y` |
| Blocks and branching | [BlockExpression], [IfExpression] | `{ make() }.field`, `if flag { a } else { b }` |
| Loops and jumps | [LoopExpression], [BreakExpression], [ContinueExpression], [ReturnExpression] | `loop { break 3; }`, `while condition { ... }`, `return value` |
| Constant contexts | [ConstValue], [Magnitude] | `123`, `COUNT`, `(-1)`, `-(2147483648i32)`, `((true))` |

语法描述构造的形状。名称解析、类型、位置可变性和[测试域](undefined-behavior.md)共同决定解析后的程序哪些有效。例如，结构体构造表达式中的标识符必须命名已声明的结构体，被调用的方法必须在接收者类型的固有 impl 中定义，赋值目标必须表示可变位置。

</details>

<details>
<summary>规则位置</summary>

| Topic | Defining chapter |
| --- | --- |
| Use declarations | [Use declarations](items/use-declarations.md) |
| Type and expression arguments | [Paths](paths.md#generic-arguments) |
| Lifetime declarations and bounds | [Lifetime parameters](items/generics.md) |
| Derive attributes | [Attribute grammar](traits-and-attributes.md#attribute-grammar) |
| Operator precedence | [Expressions](expressions.md#precedence) |
| Casts followed by comparison or shift | [Cast parsing](expressions/operator-expr.md#cast-parsing) |
| Struct construction in a condition | [If expressions](expressions/if-expr.md) |
| Expression-statement boundaries | [Statements](statements.md#statement-boundary) |
| Block tails | [Block expressions](expressions/block-expr.md) |
| Break/continue targets and loop results | [Loops](expressions/loop-expr.md#while-and-loop) |
| Never and unreachable code | [Never type](types/never.md) |
| Assignment places | [Assignment destinations](expressions/operator-expr.md#assignment-destinations) |

</details>

<a id="syntax-that-may-be-discarded-after-parsing"></a>
## 可在解析后丢弃的语法

有两种 Rust 兼容语法必须解析，但不需要后续语义处理或 LLVM IR 表示：

- [use 声明](items/use-declarations.md)：丢弃整个声明。不需要导入解析、模块加载、别名插入或导入冲突检查。
- 生命周期语法：丢弃生命周期参数声明、引用生命周期标注、显式生命周期实参、存活期约束和生命周期 where 子句。不需要生命周期名称解析、推断、省略检查或生命周期/借用检查器。

[use 兼容性](undefined-behavior.md#use-compatibility)和[生命周期有效性](undefined-behavior.md#lifetime-validity)保证排除了需要这些检查的测试。丢弃生命周期语法是内部 AST 转换；编译器无需确保删除生命周期标注后的源文本构成有效 Rust。

解析仍必须执行这些构造的语法。编译器必须保留引用类型、可变性限定符、具体类型实参（例如 `Vec::<&'a i32>` 中的 `i32`）以及所有其他 AST 结构。普通的名称、类型和位置可变性检查，以及正确的引用行为仍然是必需的。

<a id="contextual-punctuation"></a>
## 上下文标点

解析器根据当前语法上下文解释组合标点。词法分析器可以生成组合词法单元；解析器读取所需的前缀，并将剩余标点留给外层构造。

| Combined token | Context | Interpretation |
| --- | --- | --- |
| `&&` | Reference type or prefix borrow | Two `&` tokens, as in `&&i32` or `&&x` |
| `>>` | Closing nested type arguments | Two closing `>` tokens |
| `>=` | Closing type arguments before assignment | A closing `>` followed by `=` |
| `>>=` | Closing nested type arguments before assignment | Two closing `>` tokens followed by `=` |

在普通中缀表达式上下文中，这些词法单元表示对应运算符。因此，下面的变量初始化不要求在结束 `>` 和 `=` 词法单元之间添加空白：

```rust,ignore
let values: Vec<i32>=Vec::<i32>::new();
let nested: Vec<Vec<i32>>=Vec::<Vec<i32>>::new();
```
