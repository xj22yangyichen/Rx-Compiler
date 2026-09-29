<a id="lifetime-parameters-and-bounds"></a>
# 生命周期参数与约束

<a id="parameter-declarations"></a>
## 参数声明

r[items.generics.syntax]
```grammar,items
GenericParams -> `<` (GenericParam (`,` GenericParam)* `,`?)? `>`

GenericParam -> LifetimeParam

LifetimeParam -> Lifetime (`:` LifetimeBounds)?
```

函数、具名字段结构体和固有 impl 可以声明生命周期参数。函数或结构体将 [GenericParams] 放在名称之后；impl 则将该列表放在 `impl` 之后。参数列表末尾可以有一个可选的尾随逗号。每个参数声明一个生命周期，并可附带存活时间约束。

```rust,ignore
struct View<'a> {
    value: &'a i32,
}

impl<'a> View<'a> {
    fn new(value: &'a i32) -> Self {
        Self { value: value }
    }

    fn get(&self) -> &'a i32 {
        self.value
    }
}

fn choose<'a>(left: &'a i32, right: &'a i32, first: bool) -> &'a i32 {
    if first { left } else { right }
}
```

生命周期参数在声明它们的整个条目内都处于作用域中。在 `impl` 块上声明的生命周期同样适用于其关联条目内部，不过方法或关联函数可以声明额外的生命周期参数。生命周期名称占用独立的命名空间。不正确的生命周期声明、使用或约束属于未定义行为；参见[生命周期有效性保证](../undefined-behavior.md#lifetime-validity)。

<a id="lifetime-forms-and-bounds"></a>
## 生命周期形式与约束

```grammar,types
Lifetime -> LIFETIME_OR_LABEL | `'static` | `'_`

LifetimeBounds -> (Lifetime `+`)* Lifetime?

TypeParamBounds -> TypeParamBound (`+` TypeParamBound)* `+`?

TypeParamBound -> Lifetime
```

`'a` 这样的具名生命周期引用作用域内的参数。`'static` 表示覆盖整个程序的生命周期。`'_` 表示匿名生命周期，按照 Rust 的省略规则在该出现位置推断。泛型参数声明只接受普通的具名生命周期；`'static` 和 `'_` 仅在使用位置有效。

约束 `'a: 'b` 表示 `'a` 的存活时间至少覆盖 `'b`。`'a: 'b + 'c` 这样的列表同时声明两个约束。`View<'a>: 'b` 这样的类型约束表示该类型包含的引用在 `'b` 期间保持有效。这些约束中的类型是 [Type] 所描述的具体类型。

```rust,ignore
fn shorten<'long: 'short, 'short>(value: &'long i32) -> &'short i32 {
    value
}
```

<a id="where-clauses"></a>
## Where 子句

r[items.generics.where.syntax]
```grammar,items
WhereClause -> `where` (WhereClauseItem `,`)* WhereClauseItem?

WhereClauseItem -> LifetimeWhereClauseItem | TypeBoundWhereClauseItem

LifetimeWhereClauseItem -> Lifetime `:` LifetimeBounds

TypeBoundWhereClauseItem -> Type `:` TypeParamBounds?
```

where 子句在函数返回类型之后（省略返回类型时则在形参列表之后）、结构体生命周期参数之后，或 `impl` 块的目标类型之后指定生命周期约束。约束用逗号分隔，并允许可选的尾随逗号。生命周期约束可以写在这里，也可以直接写在其参数声明上。

```rust,ignore
fn shorten_where<'long, 'short>(value: &'long i32) -> &'short i32
where
    'long: 'short,
{
    value
}
```

这些产生式使用 Rust 的参数列表和约束语法。[GenericParam] 定义生命周期参数；[GenericArg] 定义路径中使用的生命周期实参和具体类型实参。支持的具名类型和可调用签名在[路径](../paths.md)中规定。
