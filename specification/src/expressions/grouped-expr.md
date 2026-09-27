# Grouped expressions

r[expr.paren.syntax]
```grammar,expressions
GroupedExpression -> `(` Expression `)`
```

A parenthesized expression has the value and type of its inner operand and preserves place status. Parentheses can force an expression context at a statement boundary and can disambiguate struct construction in conditions or a cast before `<` or `<<`.

```rust,ignore
let mut x = 5;
(x) = 10;                     // a parenthesized place remains an assignable place
let y = (1 + 2) * 3;          // grouping overrides default operator precedence
```

Empty parentheses `()` form the unit expression described by [UnitExpression].
