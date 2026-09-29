# 路径

路径通过由 `::` 分隔的片段命名值、类型或关联条目。类型路径和表达式路径共享相同的泛型实参语法（[GenericArgs]）。

[use 声明](items/use-declarations.md)为兼容 Rust 使用单独的 [UsePath] 语法。Rx 编译器不会解析其中的路径，它们也不会扩展此处定义的类型路径或表达式路径。

<a id="paths-in-expressions"></a>
## 表达式中的路径

```grammar,paths
PathInExpression -> PathExprSegment (`::` PathExprSegment)*

PathExprSegment -> PathIdentSegment (`::` GenericArgs)?

PathIdentSegment -> IDENTIFIER | `self` | `Self`
```

表达式路径在泛型实参前使用 `::`（即“涡轮鱼”语法），例如 `Box::<i32>::new`。最后一个片段可以命名值或关联条目。对这些路径的调用遵循 [CallExpression] 和 [MethodCallExpression] 的规则。

<a id="paths-in-types"></a>
## 类型中的路径

```grammar,paths
TypePath -> TypePathSegment (`::` TypePathSegment)*

TypePathSegment -> PathIdentSegment (`::`? GenericArgs)?
```

类型实参可以直接跟在片段后，也可以跟在 `::` 后，因此 `Box<i32>` 和 `Box::<i32>` 命名同一种类型。引用用户定义结构体的类型路径可以提供生命周期实参（如 `View<'a>`），也可以依赖允许的生命周期省略。

<a id="generic-arguments"></a>
## 泛型实参

```grammar,paths
GenericArgs -> `<` GenericArgList? `>`

GenericArgList -> (GenericArg `,`)* GenericArg `,`?

GenericArg -> Lifetime | Type
```

泛型实参以逗号分隔，可以有一个可选的尾随逗号；所有生命周期实参必须位于类型实参之前。在有效程序中，`Box` 和 `Vec` 恰好有一个显式具体类型实参。在 `Box::new(...)` 或 `Vec::new()` 构造函数调用中省略它属于[未定义行为](heap.md#builtin-signatures)。用户定义的函数和结构体只接受其声明的生命周期实参，而基本类型不接受泛型实参。其他泛型实参数量或种类不匹配属于编译时错误；生命周期正确性由[生命周期有效性保证](undefined-behavior.md#lifetime-validity)规定。

类型实参可以递归地包含任何受支持的具体类型。例如：

```rust,ignore
let values: Vec<Box<[i32; 4]>> = Vec::<Box<[i32; 4]>>::new();
let borrowed = Vec::<&'a i32>::new();
```

<a id="path-resolution"></a>
## 路径解析

| 路径角色 | 解析结果 |
| --- | --- |
| 值 | 绑定、常量、函数或 `self` |
| 类型 | 基本类型、已声明结构体、`Self`、`Box<T>` 或 `Vec<T>` |
| 关联条目 | 结构体路径或 `Self`，后跟已声明成员或派生的内建方法 |
| 内建关联操作 | `Box::<T>::new`、`Vec::<T>::new`，或类型路径上受支持的内建方法，例如 `Box::<i32>::clone` |

在 Rust 允许的位置，函数和方法可以使用显式生命周期实参。省略的生命周期使用受支持的 Rust 2021 推断和省略规则。测试保证显式生命周期实参（包括早期绑定和后期绑定的使用）有效。

无法解析的名称属于编译时错误。名称解析后，调用表达式会根据实参和接收者规则进行检查。提供错误的生命周期实参或无效的生命周期省略，根据[生命周期有效性保证](undefined-behavior.md#lifetime-validity)属于未定义行为。
