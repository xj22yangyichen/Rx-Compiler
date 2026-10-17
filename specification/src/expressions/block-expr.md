# Block expressions

r[expr.block.syntax]
```grammar,expressions
BlockExpression -> `{` Statements? `}`

Statements ->
      Statement+
    | Statement+ ExpressionWithoutBlock
    | ExpressionWithoutBlock
```

A block contains [statements](../statements.md) and an optional final
expression, enclosed in braces. Statements execute in source order.

A block's type and result value are determined as follows:

- If the block has a tail expression (a final expression without a semicolon), that expression determines the block's type and, when reached, its result value. A tail expression determines the block's type even if prior statements diverge (the unreachable tail expression is still type-checked against any expected type).
- If the block has no tail expression and completes normally, its type is `()` and it evaluates to `()`.
- If the block has no tail expression and does not complete normally---because every execution path exits early via a control-flow transfer (`return`, `break`, or `continue`) or by evaluating a diverging expression---the block has the [never type](../types/never.md) `!`. If some paths exit early while another path reaches the end without a tail expression, the block completes normally with type `()`.

```rust,ignore
fn select(flag: bool) -> i32 {
    let base = if flag { 10 } else { 20 };
    { base + 1 } // the inner block and function body both yield an i32
}
```

Even though the formal grammar permits parsing an un-semicoloned [ExpressionWithBlock](../expressions.md) under statement sequences, in the final position of a block it acts as the block's tail expression and determines the block's value.

Parsing of blocks and trailing statements follows the [statement boundary rules](../statements.md#statement-boundary);
local variable storage follows [storage duration](../references.md#storage-and-implementation).
