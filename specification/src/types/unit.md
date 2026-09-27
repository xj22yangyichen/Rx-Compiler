# Unit type

```grammar,types
UnitType -> `(` `)`
```

The unit type is written `()`, and its single value is described by [UnitExpression].

Its valid data uses and the function/control-flow result exceptions are defined by the [zero-sized-data rules](../undefined-behavior.md#zero-sized-data).

```rust,ignore
fn do_nothing() -> () {
    // A function with an omitted or () return type returns ()
}

let unit_val: () = ();           // Unit literal
```
