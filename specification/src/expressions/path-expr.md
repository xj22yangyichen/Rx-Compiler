# Path expressions

r[expr.path.syntax]
```grammar,expressions
PathExpression -> PathInExpression
```

A path expression resolves an unqualified value name, `self`, or an associated item such as `Point::LIMIT`. Path forms and namespace rules are specified in [Paths](../paths.md) and [Names](../names.md).

A function path serves as a call target. In valid programs, `Box` and `Vec` constructors are calls to a [PathExpression] whose container segment carries [GenericArgs]; omitting the type argument exhibits [undefined behavior](../heap.md#builtin-signatures).

## Function values

Using a function as a value, such as `let f = helper;`, is undefined behavior
under the [test guarantees](../undefined-behavior.md#test-guarantees).
This applies to ordinary functions, associated functions and methods, and
builtins. No function-value representation or diagnostic is required.
Calling a function and using its result follow the ordinary
[call rules](call-expr.md).

```rust,ignore
fn helper() -> i32 { 42 }

let result = helper();          // valid: calling a function path
// let f = helper;              // undefined behavior: function item used as a first-class value
```
