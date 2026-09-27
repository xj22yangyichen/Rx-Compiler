# Field access expressions

r[expr.field.syntax]
```grammar,expressions
FieldExpression -> Expression `.` IDENTIFIER
```

The identifier after `.` selects a declared named field; an unknown field is a compile error.

The result is a place whose mutability follows its base. A value base is evaluated once and materialized as a temporary when required. Reading the field obeys Copy/move rules. See [places and values](../expressions.md#places-and-values) and [reference storage duration](../references.md).

## Automatic dereferencing

Field access repeatedly dereferences references and `Box` until it reaches the first struct type containing named fields. These are the builtin dereference steps listed under [coercion types](../types.md#coercion-types). For `boxed: Box<S>`, `boxed.field` accesses the field of the contained `S`. Field access does not insert a borrow.

```rust,ignore
struct Point { x: i32, y: i32 }
let p = Box::<Point>::new(Point { x: 10, y: 20 });
println_i32(p.x);               // automatically dereferences Box<Point> to access field x
```

Crossing a shared reference makes the field read-only. An immutable binding
holding `&mut S` can still give mutable access to `S`'s fields. Further
adjustments follow the [dereference rules](operator-expr.md#borrow-dereference-and-assignment).
The base is evaluated only once, regardless of the number of dereferences.
