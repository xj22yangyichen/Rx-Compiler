# Literal expressions

r[expr.literal.syntax]
```grammar,expressions
LiteralExpression -> INTEGER_LITERAL | `true` | `false`
```

Literal expressions produce integer or boolean values. [INTEGER_LITERAL] defines the spelling of integer tokens.

Note that Unit `()` uses [UnitExpression] instead.

## Integer typing and range

An integer suffix fixes its type. Without a suffix, use the expected integer type when one is supplied; otherwise use `i32`. Inference requiring propagation through a binary operator, unary operator, or shared/mutable borrow is [undefined behavior](../types.md#coercion-sites-and-expected-types). Selecting the literal's type based on subsequent operands or later uses likewise results in [undefined behavior](../types.md#coercion-sites-and-expected-types). Runtime integer widths are specified in [Types](../types.md).

A leading minus forms a [NegationExpression]. The unary-minus literal case
permits the signed minimum: `-2147483648i32` uses an integer token of magnitude
2147483648 and represents the valid i32 minimum. Parenthesized negative literals such
as `-(2147483648i32)` follow the same rule.

After selecting the type by suffix, expected type, or `i32` fallback, an integer
literal whose value falls outside that type's representable range exhibits undefined behavior;
see the [integer literal range guarantee](../undefined-behavior.md#integer-literal-range).

```rust,ignore
let a = 42;             // i32 by default
let b = 42u32;          // u32 by explicit suffix
let c: usize = 42;      // usize from expected type
let min = -2147483648i32; // valid i32 minimum
```
