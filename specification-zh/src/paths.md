# Paths

A path names a value, type, or associated item through `::`-separated segments. Both type and expression paths share the same generic argument syntax ([GenericArgs]).

[Use declarations](items/use-declarations.md) have a separate [UsePath] syntax for Rust compatibility. Their paths are not resolved by the Rx compiler and do not extend the type or expression paths defined here.

## Paths in expressions

```grammar,paths
PathInExpression -> PathExprSegment (`::` PathExprSegment)*

PathExprSegment -> PathIdentSegment (`::` GenericArgs)?

PathIdentSegment -> IDENTIFIER | `self` | `Self`
```

Expression paths use `::` before generic arguments (the 'turbofish' syntax), as in `Box::<i32>::new`. Their final segment may name a value or an associated item. Calls on these paths are governed by [CallExpression] and [MethodCallExpression].

## Paths in types

```grammar,paths
TypePath -> TypePathSegment (`::` TypePathSegment)*

TypePathSegment -> PathIdentSegment (`::`? GenericArgs)?
```

Type arguments may follow a segment directly or after `::`, so `Box<i32>` and `Box::<i32>` name the same type. Type paths referencing user-defined structs may supply lifetime arguments, such as `View<'a>`, or rely on permitted lifetime elision.

## Generic arguments

```grammar,paths
GenericArgs -> `<` GenericArgList? `>`

GenericArgList -> (GenericArg `,`)* GenericArg `,`?

GenericArg -> Lifetime | Type
```

Generic arguments are comma-separated with an optional trailing comma, and any lifetime arguments must precede type arguments. In valid programs, `Box` and `Vec` have exactly one explicit concrete type argument. Omitting it in a `Box::new(...)` or `Vec::new()` constructor call is [undefined behavior](heap.md#builtin-signatures). User-defined functions and structs accept only their declared lifetime arguments, whereas primitive types accept no generic arguments. Any other mismatch in generic argument arity or kind is a compile-time error; lifetime correctness is governed by the [lifetime validity guarantee](undefined-behavior.md#lifetime-validity).

A type argument may recursively contain any supported concrete type. For example:

```rust,ignore
let values: Vec<Box<[i32; 4]>> = Vec::<Box<[i32; 4]>>::new();
let borrowed = Vec::<&'a i32>::new();
```

## Path resolution

| Path role | Resolved form |
| --- | --- |
| Value | Binding, constant, function, or `self` |
| Type | Primitive, declared struct, `Self`, `Box<T>`, or `Vec<T>` |
| Associated item | Struct path or `Self`, followed by a declared member or derived builtin method |
| Builtin associated operation | `Box::<T>::new`, `Vec::<T>::new`, or a supported builtin method on a type path, such as `Box::<i32>::clone` |

Functions and methods may use explicit lifetime arguments where Rust permits them. Omitted lifetimes use the supported Rust 2021 inference and elision rules. Tests guarantee that explicit lifetime arguments, including early-bound and late-bound uses, are valid.

Unresolved names are compile-time errors. After name resolution, call expressions are checked against argument and receiver rules. Supplying incorrect lifetime arguments or invalid lifetime elisions results in undefined behavior under the [lifetime validity guarantee](undefined-behavior.md#lifetime-validity).
