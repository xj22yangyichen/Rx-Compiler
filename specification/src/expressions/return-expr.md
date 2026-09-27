# Return expressions

r[expr.return.syntax]
```grammar,expressions
ReturnExpression -> `return` Expression?
```

`return expression` evaluates its operand and returns that value from the current function using copy/move semantics. `return;` returns the unit value `()`. Function tail expressions provide the return value when execution reaches the end of the function body without returning early. In all cases, the returned value must be compatible with the function's declared return type.

```rust,ignore
fn absolute(x: i32) -> i32 {
    if x < 0 {
        return -x;              // explicit early return
    }
    x                           // tail expression return
}
```

See [Never type](../types/never.md) for typing and reachability.
