# Statements and bindings

```grammar,statements
Statement -> `;` | LetStatement | ExpressionStatement
```

## Identifier bindings

```grammar,statements
LetStatement -> `let` IdentifierBinding (`:` Type)? `=` Expression `;`

IdentifierBinding -> `mut`? IDENTIFIER
```

A `let` statement binds a single identifier to the evaluated result of an initializer expression. Specifying `mut` makes the binding mutable. The initializer may produce any supported value, including a reference formed by a borrow expression.

Binding visibility and shadowing follow [Names](names.md#scope-rules). A `let` binding that collides with a visible unqualified constant name results in undefined behavior; see the [constant-name collision guarantee](undefined-behavior.md#constant-name-collisions).

The optional type annotation and initializer expression are governed by [local type inference](types.md#inference) and [coercion rules](types.md#conversions-and-coercions). Subsequent assignments to the binding are governed by [assignment destinations](expressions/operator-expr.md#assignment-destinations).

## Expression statements

```grammar,statements
ExpressionStatement ->
      ExpressionWithoutBlock `;`
    | ExpressionWithBlock `;`?
```

An expression statement evaluates an expression and discards its result while
preserving its effects. An empty `;` statement has no effect.

[ExpressionWithoutBlock] requires a semicolon. [ExpressionWithBlock]
(`{ ... }`, `if`, `while`, or `loop`) may omit it. When the semicolon is omitted,
the expression's evaluated type must be compatible with the unit type `()`, or
the expression must diverge. A block's final expression follows the
[tail-expression rules](expressions/block-expr.md).

```rust,ignore
fn show(flag: bool) {
    if flag { print_i32(1); }    // unit result: semicolon optional
    if flag { 1 } else { 2 };   // integer result: semicolon discards it
}
```

## Statement boundary

When parsing an expression statement, an expression with an outer block form terminates the statement immediately rather than greedily consuming any subsequent infix operator. In an initializer or other value-expression context, the expression continues normally. Parentheses can force an ordinary expression context.

An attached `else` or `else if` clause remains part of its `if` expression, and postfix field accesses or method calls may continue directly after a completed block expression. Other expression statements extend until their terminating semicolon; alternatively, an enclosing block may conclude with a tail expression immediately preceding the closing `}`. For example, `{ make() }.value;` is a valid field-access expression statement provided `make()` returns a struct with a `value` field.

```rust,ignore
let value = if true { 10 } else { 20 } - 1; // initializer is the whole subtraction
if true {} else {} -1;                      // if statement, then -1 expression statement
(if true { 10 } else { 20 }) - 1;           // one expression statement
```

A conforming compiler must implement these statement boundaries and adhere to the grammar defined above.
