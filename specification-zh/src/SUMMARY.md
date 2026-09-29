# Rx 编译器规范

[引言](introduction.md)

- [语言范围与测试保证](undefined-behavior.md)

# 源语言

- [记法](notation.md)
- [词法结构](lexical-structure.md)
    - [输入格式](input-format.md)
    - [关键字](keywords.md)
    - [标识符](identifiers.md)
    - [注释](comments.md)
    - [空白字符](whitespace.md)
    - [词法单元](tokens.md)
- [解析器约定](grammar.md)
- [Crate 与源文件](crates-and-source-files.md)
- [名称与作用域](names.md)
    - [路径](paths.md)
- [条目](items.md)
    - [use 声明](items/use-declarations.md)
    - [函数](items/functions.md)
    - [结构体](items/structs.md)
    - [常量条目](items/constant-items.md)
    - [实现](items/implementations.md)
    - [关联条目](items/associated-items.md)
    - [生命周期参数与约束](items/generics.md)
- [Trait 与属性](traits-and-attributes.md)
    - [内置 Trait](builtin-traits.md)
- [语句与绑定](statements.md)
- [类型与推断](types.md)
    - [数组类型](types/array.md)
    - [引用类型](types/pointer.md)
    - [Box 与 Vec 类型](types/heap.md)
    - [单元类型](types/unit.md)
    - [Never 类型](types/never.md)
- [表达式与求值](expressions.md)
    - [字面量表达式](expressions/literal-expr.md)
    - [路径表达式](expressions/path-expr.md)
    - [块表达式](expressions/block-expr.md)
    - [运算符表达式](expressions/operator-expr.md)
    - [分组表达式](expressions/grouped-expr.md)
    - [单元表达式](expressions/tuple-expr.md)
    - [数组与索引表达式](expressions/array-expr.md)
    - [结构体表达式](expressions/struct-expr.md)
    - [调用表达式](expressions/call-expr.md)
    - [方法调用表达式](expressions/method-call-expr.md)
    - [字段访问表达式](expressions/field-expr.md)
    - [循环表达式](expressions/loop-expr.md)
    - [if 表达式](expressions/if-expr.md)
    - [return 表达式](expressions/return-expr.md)
- [常量上下文](const_eval.md)

# 值与执行

- [引用与存储期](references.md)
- [Box 与 Vec](heap.md)
- [内置 I/O 与程序入口](undefined-behavior/builtin.md)
- [后端](backend.md)

# 附录

- [语法汇总](grammar-summary.md)
<!-- - [发布验证](open-decisions.md) -->
