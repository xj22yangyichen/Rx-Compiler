# Loop expressions

## While and loop

```grammar,expressions
LoopExpression -> InfiniteLoopExpression | PredicateLoopExpression

InfiniteLoopExpression -> `loop` BlockExpression

PredicateLoopExpression -> `while` Conditions BlockExpression

BreakExpression -> `break` Expression?

ContinueExpression -> `continue`
```

`while condition { ... }` reevaluates a `bool` condition before each iteration and evaluates to the unit value `()`. Parentheses around the condition are optional. `loop { ... }` repeats until control flow leaves it; `break` values determine the loop expression's result type. The body block of either form must be type-compatible with the unit type `()`; a diverging body is compatible. A non-unit trailing expression in the body block does not become the value of the loop expression. Thus, `while false { 1 }` and `loop { 1 }` cause compile-time type errors, whereas a `loop` expression can yield a value via `break value;`.

`break;` exits the nearest loop with unit. A value-bearing break requires a `loop` target. The loop's expected type applies to all its break values through [one-to-one coercions](../types.md#coercion-sites-and-expected-types); without one, [LUB coercion](../types.md#least-upper-bound-coercions) finds their common type. `continue;` starts the next iteration of the nearest enclosing loop (including reevaluating a `while` condition). Using `break` or `continue` outside a loop is a compile error.

A break or continue appearing in a while condition must target a loop nested inside that condition. Targeting the containing while or any outer loop is a compile error. For example, `while loop { break false; } {}` is allowed. Syntactic loop nesting determines the target.

```rust,ignore
fn until(limit: i32) -> i32 {
    let mut n = 0;
    loop {
        if n == limit { break n; }
        n += 1;
    }
}
```

See [Never type](../types/never.md#loop-expressions) for when a `loop` expression
has the never type, and [Unreachable code](../types/never.md#unreachable-code) for
checks that still apply after control flow leaves a block.
