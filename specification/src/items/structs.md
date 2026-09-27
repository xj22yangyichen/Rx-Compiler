# Structs

r[items.struct.syntax]
```grammar,items
Struct -> OuterAttribute* StructStruct

StructStruct -> `struct` IDENTIFIER GenericParams? WhereClause? `{` StructFields? `}`

StructFields -> StructField (`,` StructField)* `,`?

StructField -> IDENTIFIER `:` Type
```

A struct declaration defines a name, optional lifetime parameters and where-clause bounds, and a brace-delimited list of named fields. The field list may include an optional trailing comma after the final field. A struct may have [derive attributes](../traits-and-attributes.md#attribute-grammar).

Lifetime parameters describe references stored in the struct, as in `struct View<'a> { value: &'a i32 }`. A type path supplies the corresponding arguments with `View<'a>` or uses Rust's permitted lifetime elision. See [lifetime parameters](generics.md) and [lifetime validity](../references.md#lifetime-validity).

A struct is a nominal type with explicitly typed, named fields. Its fields are accessible throughout the source compilation unit. [Name rules](../names.md#scope-rules) define field uniqueness and the meaning of `Self` in its declaration. For example, in `struct Node { children: Vec<Self> }`, the type `Self` refers to the `Node` struct being defined.

Struct definitions must have [finite layouts](../types.md#recursive-types). Struct declarations with empty braces (zero-sized structs) are syntactically permitted, and operations on zero-sized data follow the [zero-sized-data rules](../undefined-behavior.md#zero-sized-data). The [backend contract](../backend.md#data-layout) governs implementation layout. Values are created with [struct expressions](../expressions/struct-expr.md) and accessed with [field expressions](../expressions/field-expr.md).

```rust,ignore
#[derive(Clone, Copy, PartialEq, Eq)]
struct Point {
    x: i32,
    y: i32,
}
```
