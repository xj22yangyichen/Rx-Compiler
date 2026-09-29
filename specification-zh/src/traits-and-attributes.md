# Traits and attributes

Rx supports derive attributes for four [builtin traits](builtin-traits.md): `Copy`, `Clone`, `PartialEq`, and `Eq`. User-defined traits, trait bounds, and explicit trait implementations are unsupported.

## Attribute grammar

```grammar,attributes
OuterAttribute -> `#` `[` DeriveAttribute `]`

DeriveAttribute -> `derive` `(` (DeriveName (`,` DeriveName)* `,`?)? `)`

DeriveName -> `Copy` | `Clone` | `PartialEq` | `Eq`
```

This is the complete attribute grammar. Zero or more outer attributes ([OuterAttribute]) may precede a top-level named-field [Struct]. Attributes on other constructs, inner attributes (`#![...]`), and other attribute names or derive names are unsupported.

Derive lists may be empty or have a trailing comma. All derive attributes on a struct contribute to the same set of requested traits; specifying duplicate traits within the same attribute or across multiple attributes on the same struct is a compile-time error. Each requested trait must satisfy its [requirements](builtin-traits.md#trait-capability-summary).

```rust,ignore
#[derive(Clone)]
#[derive(Copy, PartialEq, Eq,)]
struct Point { x: i32, y: i32 }
```
