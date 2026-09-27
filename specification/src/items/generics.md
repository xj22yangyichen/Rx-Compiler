# Lifetime parameters and bounds

## Parameter declarations

r[items.generics.syntax]
```grammar,items
GenericParams -> `<` (GenericParam (`,` GenericParam)* `,`?)? `>`

GenericParam -> LifetimeParam

LifetimeParam -> Lifetime (`:` LifetimeBounds)?
```

Functions, named-field structs, and inherent impls may declare lifetime
parameters. A function or struct places [GenericParams] after its name; an impl
places the list after `impl`. The parameter list may end with an optional trailing comma.
Each parameter declares a lifetime, optionally accompanied by outlives bounds.

```rust,ignore
struct View<'a> {
    value: &'a i32,
}

impl<'a> View<'a> {
    fn new(value: &'a i32) -> Self {
        Self { value: value }
    }

    fn get(&self) -> &'a i32 {
        self.value
    }
}

fn choose<'a>(left: &'a i32, right: &'a i32, first: bool) -> &'a i32 {
    if first { left } else { right }
}
```

Lifetime parameters remain in scope throughout the item in which they are declared.
Lifetimes declared on an `impl` block also apply within its associated items, though
a method or associated function may declare additional lifetime parameters. Lifetime names
occupy their own namespace. Incorrect lifetime declarations, uses, or bounds are
undefined behavior; see the [lifetime validity guarantee](../undefined-behavior.md#lifetime-validity).

## Lifetime forms and bounds

```grammar,types
Lifetime -> LIFETIME_OR_LABEL | `'static` | `'_`

LifetimeBounds -> (Lifetime `+`)* Lifetime?

TypeParamBounds -> TypeParamBound (`+` TypeParamBound)* `+`?

TypeParamBound -> Lifetime
```

A named lifetime such as `'a` refers to an in-scope parameter. `'static` denotes
the lifetime covering the entire program. `'_` represents an anonymous lifetime
inferred according to Rust's elision rules at that occurrence. Generic parameter declarations
only accept ordinary named lifetimes; `'static` and `'_` are only valid at use sites.

The bound `'a: 'b` means that `'a` outlives `'b`. A list such as
`'a: 'b + 'c` states both bounds. A type bound such as `View<'a>: 'b` means that
references contained in the type remain valid for `'b`. The types in these
bounds are the concrete types described by [Type].

```rust,ignore
fn shorten<'long: 'short, 'short>(value: &'long i32) -> &'short i32 {
    value
}
```

## Where clauses

r[items.generics.where.syntax]
```grammar,items
WhereClause -> `where` (WhereClauseItem `,`)* WhereClauseItem?

WhereClauseItem -> LifetimeWhereClauseItem | TypeBoundWhereClauseItem

LifetimeWhereClauseItem -> Lifetime `:` LifetimeBounds

TypeBoundWhereClauseItem -> Type `:` TypeParamBounds?
```

A where clause specifies lifetime bounds following a function's return type
(or parameter list if omitted), a struct's lifetime parameters, or an `impl` block's
target type. Bounds are comma-separated with an optional trailing comma. A lifetime
bound may be written here or directly on its parameter declaration.

```rust,ignore
fn shorten_where<'long, 'short>(value: &'long i32) -> &'short i32
where
    'long: 'short,
{
    value
}
```

These productions use Rust's parameter-list and bound syntax. [GenericParam]
defines lifetime parameters; [GenericArg] defines the lifetime and concrete
type arguments used in paths. The supported named types and callable
signatures are specified in [Paths](../paths.md).
