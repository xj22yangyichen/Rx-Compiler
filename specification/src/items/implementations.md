# Implementations

r[items.impl.syntax]
```grammar,items
Implementation -> InherentImpl

InherentImpl -> `impl` GenericParams? Type WhereClause? `{` AssociatedItem* `}`
```

An inherent impl defines associated items for a user-defined named-field struct. The implementing [Type] must resolve to that struct; specifying any other kind of target type is a compile-time error. Parentheses enclosing the type retain their standard grouping semantics; thus, `impl S { ... }` and `impl (S) { ... }` target the same struct.

An `impl` block may declare lifetime parameters immediately after the `impl` keyword, as in `impl<'a> View<'a> { ... }`. These parameters remain in scope throughout all associated items in the block. Methods and associated functions may declare additional lifetime parameters. A [WhereClause] before the opening brace specifies lifetime bounds.

Multiple `impl` blocks for the same struct are supported. Their member namespaces and duplicate-name restrictions are governed by [Names](../names.md#scope-rules). Permitted items within an implementation block are defined in [Associated items](associated-items.md), and method receivers are described in [Function receivers](functions.md#receivers).

```rust,ignore
impl Point {
    const ZERO: i32 = 0;

    fn total(&self) -> i32 {
        self.x + self.y
    }

    fn translate(&mut self, amount: i32) {
        self.x += amount;
        self.y += amount;
    }
}
```
