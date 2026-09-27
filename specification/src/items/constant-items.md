# Constant items

r[items.const.syntax]
```grammar,items
ConstantItem -> `const` IDENTIFIER `:` Type `=` ConstValue `;`
```

A constant declaration specifies a name, an explicit type, and an initializer. The initializer expression must evaluate to a [ConstValue] conforming to the [constant-context rules](../const_eval.md). A constant initializer may reference other constant items, subject to the same rules and an acyclic-dependency requirement. Referencing a constant in an ordinary expression evaluates to its defined value.

Constants are declared at the top level or as associated items in an inherent impl. The [name lookup rules](../names.md#scope-rules) permit forward references, allowing a constant to be used before its textual declaration.

```rust,ignore
const LIMIT: i32 = 100;
const NEGATIVE: i32 = (-1);
```
