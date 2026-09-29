<a id="structs"></a>
# 结构体

r[items.struct.syntax]
```grammar,items
Struct -> OuterAttribute* StructStruct

StructStruct -> `struct` IDENTIFIER GenericParams? WhereClause? `{` StructFields? `}`

StructFields -> StructField (`,` StructField)* `,`?

StructField -> IDENTIFIER `:` Type
```

结构体声明定义名称、可选的生命周期参数和 where 子句约束，以及用花括号界定的具名字段列表。字段列表的最后一个字段后可以有一个可选的尾随逗号。结构体可以带有 [derive 属性](../traits-and-attributes.md#attribute-grammar)。

生命周期参数描述结构体中存储的引用，例如 `struct View<'a> { value: &'a i32 }`。类型路径通过 `View<'a>` 提供相应实参，或使用 Rust 允许的生命周期省略形式。参见[生命周期参数](generics.md)和[生命周期有效性](../references.md#lifetime-validity)。

结构体是一种名义类型，具有显式类型的具名字段。其字段在整个源代码编译单元内都可访问。[名称规则](../names.md#scope-rules)定义字段名称的唯一性以及声明中 `Self` 的含义。例如，在 `struct Node { children: Vec<Self> }` 中，类型 `Self` 指代正在定义的 `Node` 结构体。

结构体定义必须具有[有限布局](../types.md#recursive-types)。语法上允许使用空花括号声明结构体（零大小结构体），对零大小数据的操作遵循[零大小数据规则](../undefined-behavior.md#zero-sized-data)。实现的布局受[后端契约](../backend.md#data-layout)约束。值通过[结构体表达式](../expressions/struct-expr.md)创建，通过[字段表达式](../expressions/field-expr.md)访问。

```rust,ignore
#[derive(Clone, Copy, PartialEq, Eq)]
struct Point {
    x: i32,
    y: i32,
}
```
