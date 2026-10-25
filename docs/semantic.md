# Semantic 语义分析

语法分析可以确认 `x + true` 是一个二元表达式；语义分析还需要确定 `x` 是否已声明、两个操作数的类型是否允许相加，以及结果能否用于所在上下文。

实现名字解析、类型检查、局部类型推断、可变性检查、常量求值、结构体派生能力与递归布局检查，以及函数、方法和控制流检查。

## 文件

源码位于 `src/semantic/`，命名空间为 `rx::semantic`。

| 文件 | 职责 |
| --- | --- |
| [analyzer.h](../src/semantic/analyzer.h) | `Analyzer`、符号信息、位置和循环上下文各种信息的声明 |
| [analyzer.cpp](../src/semantic/analyzer.cpp) | 分析入口、声明收集、类型解析、派生与布局检查、常量求值 |
| [type.h](../src/semantic/type.h)、[type.cpp](../src/semantic/type.cpp) | 辅助工具，包括语义类型表示、整数类型、类型相同的判定 |
| [check-stmt.cpp](../src/semantic/check-stmt.cpp) | 函数体、块、语句、`let` 等的处理 |
| [check-expr.cpp](../src/semantic/check-expr.cpp) | 表达式类型、位置分析、调用、类型转换和公共类型计算 |

命令行入口：

```sh
./target/frontend/rxc --stage semantic program.rx
```

构建好 `AST` 后，调用 `semantic::analyze(crate, file, diagnostics)`，创建一个 `Analyzer`，调用其 `run` 方法。

错误通过前一阶段实现的 `Diagnostic` 记录，由 `main` 输出到 stderr。无错误时退出码为 0，有为 1。

## 工作流程

收集结构体声明 -> 收集函数和常量声明 -> 收集 impl 关联项 -> 检查派生能力并解析字段类型 -> 检查 main 入口 -> 检查签名并求值常量 -> 检查函数体和方法体 -> 返回检查结果

| 步骤 | 主要函数 | 作用 |
| --- | --- | --- |
| 声明收集 | `collectStructs`、`collectValues`、`collectImpls` | 建立名字到声明的映射，检查重名和非法关联项 |
| 派生检查 | `checkDerives` | 解析并缓存字段类型，检查 `Copy/Clone/PartialEq/Eq` 的要求 |
| 布局检查 | `checkLayout` | 检测直接或经数组包含自身的无限大小类型 |
| 入口检查 | `checkEntry` | 要求 `main` 存在，无值参数和泛型参数，返回 unit |
| 签名检查 | `checkSignatures` | 解析参数与返回类型，检查参数重名，求值常量 |
| 函数体检查 | `checkFunction` | 建立参数作用域和返回类型上下文，递归检查函数体 |

先收集声明使函数体可以引用后面定义的函数和结构体，也支持递归调用。
结构体字段类型在声明所属上下文中解析并缓存，避免使用字段时误用调用处的 `Self`。

总共遍历 `item` 5次，前3次是声明收集，然后 `checkDerives` 遍历 `structs_` 两轮，第一轮检查派生能力之间的依赖，第二轮解析字段类型，缓存，并验证字段是否满足派生能力要求，接着进行布局和入口检查，最后2次遍历分别检查签名检查、常量求值和函数体检查、错误记录。

## Visitor Pattern

| 访问入口 | 输入 | 返回结果 |
| --- | --- | --- |
| `resolveType` | `ast::Type` | 解析后的语义类型 |
| `checkExpr` | `ast::Expr` | 表达式类型 |
| `checkStmt` | `ast::Stmt` | 语句检查结果，并可传出发散信息 |
| `checkPlace` | `ast::Expr` | 可选的位置及可变性信息 |
| `evalConst` | `ast::Expr` | 可选的常量类型和值 |

## 类型与上下文

`ast::Type` 表示源码中的类型写法，`semantic::Type` 表示解析后的语义类型。
后者支持四种整数、`bool`、unit、never、数组、引用、`Box`、`Vec` 和结构体。
引用或容器通过 `target` 指向内部类型，数组另存长度，结构体保存声明指针。

类型对象由 `Analyzer::arena_` 中的 `std::deque<Type>` 持有。
分析过程中传递 `const Type*`；类型相等由 `sameType` 递归比较结构，遇到结构体时比较声明身份。

`checkExpr(expression, expected)` 接收可选的期望类型，并返回表达式类型。
例如 `let count: usize = 3;` 会把 `usize` 传给初始化表达式，
使无后缀整数 `3` 采用该类型。缺少期望类型时，普通无后缀整数默认为 `i32`。
期望类型并不代替兼容性检查，初始化、实参和返回值等位置还需验证类型转换。

| 状态 | 用途 |
| --- | --- |
| `structs_`、`functions_`、`constants_` | 全局声明与名字查找 |
| `scopes_` | 局部作用域栈，从内向外查找局部变量 |
| `self_struct_` | 当前 `Self` 对应的结构体 |
| `return_type_` | 当前函数的返回类型 |
| `loops_` | 循环是否允许带值 `break`、期望类型和已收集的退出类型 |
| `condition_barriers_` | 检查循环条件中的跳转是否越过允许的循环范围 |

进入块时压入作用域，退出时弹出；检查函数或方法时设置相应上下文。
调用方法、解析其签名中的 `Self` 时，需要使用被调用方法所属结构体的上下文。

## 可变性、转换与控制流

表达式的类型不能单独说明它是否可写，因此另用 `Place` 表示位置：

- `type`：位置中存储的类型。
- `is_mutable`：该位置是否允许修改。
- `is_borrowed_shared`：访问路径是否经过共享借用。

`checkPlace` 处理路径、字段、索引和解引用，供赋值、可变借用和方法调用使用。
区分后两个标志，可以表达“不可变绑定中的可变引用仍能修改目标”，也可以阻止通过已经经过共享借用的路径继续获得可写访问。

`adjustTo` 判断能否转换到目标类型，`coerce` 在不兼容时产生诊断。
支持的情况包括相同类型、never 到目标类型、可变引用到共享引用，以及符合可变性约束的引用和 `Box` 解引用转换。
没有期望类型时，`lubOf` 为 `if` 分支、数组元素或多个 `break` 值寻找公共类型。

块检查分别计算类型和是否发散，即是否无法正常结束：

- 有尾表达式时，块类型取尾表达式类型，即使前面的语句已经发散。
- 无尾表达式且发散时，块类型为 `!`；否则为 `()`。
- 遇到发散语句后仍检查后续代码，保证不可达代码中的静态错误不会被跳过。

## 常量与方法调用

`evalConst` 处理整数和布尔字面量、常量路径及一元负号；括号在构建 AST 时已归一化。不支持的常量表达式形式会产生诊断。
`constantValue` 用 `const_cache_` 缓存结果，用 `const_stack_` 检测常量依赖环。
数组长度和重复次数以 `usize` 为期望类型求值。

普通调用检查被调函数、参数数量、参数类型和返回类型。
方法调用逐层尝试接收者类型及其共享、可变引用候选，必要时继续解引用，查找内建方法或结构体固有方法，再检查接收者可变性和实参类型。

## 验证

运行官方语义测试：

```sh
make test STAGE=semantic
```

测试框架先构建 `rxc`，再根据每个用例的 `compilation_success` 检查退出码。

当前语义分析读取 AST 并收集诊断，没有把推导类型写回 AST，也没有导出持久化类型表。
类型对象随本次 `Analyzer` 销毁。后续 IR 阶段若要复用类型和名字解析结果，需要增加生命周期覆盖代码生成阶段的分析结果结构。
