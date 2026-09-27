# Reference types

```grammar,types
ReferenceType -> `&` Lifetime? `mut`? TypeNoBounds
```

A reference type begins with `&`, an optional [Lifetime], optional `mut`, and the referenced type. Examples are `&i32`, `&'a i32`, `&'a mut Vec<i32>`, and `&'_ i32`.

Shared and mutable references are distinct types. Struct fields use these same reference forms. Lifetime annotations and elision follow the [lifetime validity rules](../references.md#lifetime-validity); tests guarantee that those rules and borrow validity hold. Ordinary type and place-mutability checks are required.

In a type context, `&&T` is parsed as two nested reference type constructors via contextual splitting of the `&&` token. The same rule applies recursively inside array types and type arguments to `Box` and `Vec`.

```rust,ignore
let r: &i32 = &x;              // shared reference
let m: &mut i32 = &mut x;      // mutable reference
let nested: &&i32 = &r;        // parsed as &(&i32) via contextual splitting of &&
```
