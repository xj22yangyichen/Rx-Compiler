<a id="field-access-expressions"></a>
# 字段访问表达式

r[expr.field.syntax]
```grammar,expressions
FieldExpression -> Expression `.` IDENTIFIER
```

`.` 后的标识符选择一个已声明的具名字段；未知字段会导致编译错误。

结果是一个位置，其可变性遵循基表达式。作为基表达式的值只求值一次，并在需要时物化为临时值。读取字段遵循 Copy/移动规则。参见[位置与值](../expressions.md#places-and-values)和[引用存储期](../references.md)。

<a id="automatic-dereferencing"></a>
## 自动解引用

字段访问会反复解引用引用和 `Box`，直到遇到第一个包含具名字段的结构体类型。这些是[强制转换类型](../types.md#coercion-types)中列出的内建解引用步骤。对于 `boxed: Box<S>`，`boxed.field` 访问其包含的 `S` 的字段。字段访问不会插入借用。

```rust,ignore
struct Point { x: i32, y: i32 }
let p = Box::<Point>::new(Point { x: 10, y: 20 });
println_i32(p.x);               // 自动解引用 Box<Point> 以访问字段 x
```

经过共享引用会使字段变为只读。持有 `&mut S` 的不可变绑定仍可提供对 `S` 字段的可变访问。进一步的调整遵循[解引用规则](operator-expr.md#borrow-dereference-and-assignment)。无论解引用多少次，基表达式都只求值一次。
