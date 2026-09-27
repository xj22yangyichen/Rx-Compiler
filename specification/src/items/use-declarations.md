# Use declarations

```grammar,items
UseDeclaration -> `use` UseTree `;`

UseTree ->
      (UsePath? `::`)? (`*` | `{` (UseTree (`,` UseTree)* `,`?)? `}`)
    | UsePath (`as` (IDENTIFIER | `_`))?

UsePath -> `::`? UsePathSegment (`::` UsePathSegment)*

UsePathSegment -> IDENTIFIER | `self` | `super` | `crate`
```

Use declarations are accepted as top-level items so that the same test source
can be compiled with Rust and the course support crate. For example:

```rust,ignore
use rx::core::*;

fn main() {
    println_i32(42);
}
```

The syntax accepts individual paths, `*` globs, nested brace groups with optional
trailing commas, and `as` aliases. For example, `use rx::core::println_i32;` and
`use rx::core::{get_i32, println_i32};` are also accepted. Use declarations cannot
appear in expression blocks or `impl` blocks, and they do not accept visibility modifiers or
attributes.

After parsing, the compiler may discard the entire declaration. The compiler does not
resolve import paths, load crates or modules, introduce names or aliases into any scope,
check for import conflicts, or emit LLVM IR for the declaration. Builtins such as
`println_i32` are already available in Rx, with or without a `use` declaration.

The [use compatibility guarantee](../undefined-behavior.md#use-compatibility)
ensures that tests do not depend on additional import semantics. The extended path syntax
accepted in use declarations (including multi-segment paths, leading `::`, `crate`, and `super`)
does not apply to [type and expression paths](../paths.md). See also
[syntax that may be discarded after parsing](../grammar.md#syntax-that-may-be-discarded-after-parsing).
