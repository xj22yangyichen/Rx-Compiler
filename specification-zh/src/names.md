# Names and scopes

## Scope rules

Top-level functions, structs, constants, and inherent associated items introduce names. Parameters and `let` statements introduce local bindings. Fields belong to their enclosing struct's namespace and are resolved through the receiver's type.

[Use declarations](items/use-declarations.md) do not introduce names in Rx and participate in neither name resolution nor collision checks. Imported paths and aliases may be discarded after parsing; the builtin environment is independent of these declarations.

Top-level and associated-item lookup is independent of declaration order. A local binding is visible only after its initializer and until the end of its block, subject to shadowing. Constant initializers, array lengths, and array-repeat lengths may use constant-item paths under the [constant-context rules](const_eval.md), which also require acyclic constant dependencies.

`self` denotes a method receiver. `Self` denotes the struct being declared or the target type of the current inherent implementation, including within its associated items. Neither is an ordinary user-defined identifier, and `Self` cannot be used in top-level functions declared outside of an `impl` block.

Lifetime names occupy a separate namespace governed by [lifetime parameters](items/generics.md) and the [lifetime validity guarantee](undefined-behavior.md#lifetime-validity).

### Type and value namespaces

Types and values use separate namespaces:

| Context | Namespace |
| --- | --- |
| Struct name, primitive type, `Box`, `Vec`, or `Self` in a type position | Type |
| Function, constant, local binding, or `self` in an expression | Value |
| Prefix of `Type::member` | Type |
| Struct field | That struct's field namespace |

A named-field struct is not a callable value constructor; therefore, a struct and a function may share the same name without collision.

## Name collisions

Top-level names must be unique within their namespace. Fields within a struct and parameters within a function must also be unique. All inherent implementations of a struct share a single associated-value namespace; defining duplicate methods, associated functions, or associated constants is a compile-time error, regardless of their signatures or whether they are declared across different `impl` blocks. A struct field and an inherent method may share the same name because they reside in separate namespaces.

Successive `let` bindings may shadow earlier locals, parameters, and non-protected global functions. Lookup resolves to the nearest enclosing binding; if that binding is not callable, an invocation is a compile-time error rather than falling back to an outer shadowed function. The initializer expression of a `let` binding is evaluated in the scope preceding the newly introduced binding.

A `let` or ordinary parameter that collides with a visible unqualified constant is undefined behavior under the centralized [constant-name collision guarantee](undefined-behavior.md#constant-name-collisions).

## Protected builtin names

The following names are protected in their respective namespaces:

| Namespace | Protected names |
| --- | --- |
| Type | `i32`, `u32`, `isize`, `usize`, `bool`, `Box`, `Vec`, `Copy`, `Clone`, `PartialEq`, `Eq` |
| Value | `get_i32`, `print_i32`, `println_i32` |
| Derive entry | `Copy`, `Clone`, `PartialEq`, `Eq` |

These protected names remain valid lexical identifiers. They may be used in other namespaces or as field and associated-item names (for example, a field named `Vec` is valid). Because inherent implementations can only be declared for user-defined structs, inherent methods cannot be added to protected builtin types.

The following collisions are undefined behavior under the [test guarantees](undefined-behavior.md#test-guarantees), with no required diagnostic or behavior:

- Declaring a struct named `u32`, `isize`, `usize`, `bool`, `Box`, `Vec`, `Copy`, `Clone`, `PartialEq`, or `Eq`.
- Declaring a local binding (`let` or ordinary parameter) named `get_i32`, `print_i32`, or `println_i32`.

Other redeclarations of protected builtins within their respective namespaces are compile-time errors, including defining a struct named `i32` or declaring a top-level function or constant with a protected I/O name.
