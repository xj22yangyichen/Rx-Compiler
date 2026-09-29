<a id="implementations"></a>
# 实现

r[items.impl.syntax]
```grammar,items
Implementation -> InherentImpl

InherentImpl -> `impl` GenericParams? Type WhereClause? `{` AssociatedItem* `}`
```

固有 impl 为用户定义的具名字段结构体定义关联条目。实现所针对的 [Type] 必须解析为该结构体；指定任何其他种类的目标类型都会导致编译时错误。包围类型的括号保留其标准分组语义；因此，`impl S { ... }` 和 `impl (S) { ... }` 针对同一个结构体。

`impl` 块可以在 `impl` 关键字之后立即声明生命周期参数，例如 `impl<'a> View<'a> { ... }`。这些参数在块中的所有关联条目内都处于作用域中。方法和关联函数可以声明额外的生命周期参数。左花括号前的 [WhereClause] 指定生命周期约束。

支持为同一个结构体编写多个 `impl` 块。它们的成员命名空间和重复名称限制受[名称](../names.md#scope-rules)规则约束。实现块内允许的条目在[关联条目](associated-items.md)中定义，方法接收者则在[函数接收者](functions.md#receivers)中描述。

```rust,ignore
impl Point {
    const ZERO: i32 = 0;

    fn total(&self) -> i32 {
        self.x + self.y
    }

    fn translate(&mut self, amount: i32) {
        self.x += amount;
        self.y += amount;
    }
}
```
