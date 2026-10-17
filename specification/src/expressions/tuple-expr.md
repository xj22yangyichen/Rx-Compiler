# Unit expressions

```grammar,expressions
UnitExpression -> `(` `)`
```

The expression `()` constructs the unit value.

The unit value `()` is the result of a function or control-flow expression that completes without producing a return value. Storing unit values in variables, passing them as arguments, or embedding them in aggregate or heap data exhibits undefined behavior under the [zero-sized-data guarantee](../undefined-behavior.md#zero-sized-data).

```rust,ignore
fn log_status() {
    // implicitly evaluates to ()
}

log_status();                    // evaluates to the unit value () and discards it
```
