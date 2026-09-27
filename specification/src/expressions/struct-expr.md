# Struct expressions

r[expr.struct.syntax]
```grammar,expressions
StructExpression -> PathInExpression `{` StructExprFields? `}`

StructExprFields -> StructExprField (`,` StructExprField)* `,`?

StructExprField -> IDENTIFIER `:` Expression
```

A struct expression consists of a type path followed by a brace-delimited list of `field: expression` initializers. The path must resolve in the type namespace to a declared named-field struct. The supported forms include a struct name with permitted lifetime arguments or elisions (e.g., `View::<'a> { value: x }` or `View { value: x }`), as well as `Self` within an `impl` block. [Path resolution](../paths.md#path-resolution) identifies the named struct, while lifetime validity is guaranteed by the test suite.

Each declared field must appear exactly once, initialized with an expression whose type is compatible with the declared field type. Missing fields, duplicate fields, unknown fields, and incompatible values result in compile errors. Field initializers are evaluated in source order according to the [ordinary evaluation order](../expressions.md#evaluation-order). At condition boundaries, the [condition parsing rule](if-expr.md) distinguishes the opening brace of a struct initializer from that of a control-flow body block.

```rust,ignore
struct Point { x: i32, y: i32 }
fn main() {
    let point = Point { y: 2, x: 1 };
    println_i32(point.x);
}
```
