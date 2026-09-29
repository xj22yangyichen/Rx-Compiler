<a id="reference-types"></a>
# 引用类型

```grammar,types
ReferenceType -> `&` Lifetime? `mut`? TypeNoBounds
```

引用类型以 `&` 开始，后接可选的 [Lifetime]、可选的 `mut` 以及所引用的类型。例如 `&i32`、`&'a i32`、`&'a mut Vec<i32>` 和 `&'_ i32`。

共享引用和可变引用是不同的类型。结构体字段使用同样的引用形式。生命周期标注和省略遵循[生命周期有效性规则](../references.md#lifetime-validity)；测试保证这些规则成立且借用有效。必须进行普通的类型和位置可变性检查。

在类型上下文中，通过按上下文拆分 `&&` 词法单元，`&&T` 被解析为两个嵌套的引用类型构造器。同一规则递归地应用于数组类型内部，以及 `Box` 和 `Vec` 的类型实参内部。

```rust,ignore
let r: &i32 = &x;              // 共享引用
let m: &mut i32 = &mut x;      // 可变引用
let nested: &&i32 = &r;        // 通过按上下文拆分 &&，解析为 &(&i32)
```
