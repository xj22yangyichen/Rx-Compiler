# Types and inference

```grammar,types
Type -> TypeNoBounds

TypeNoBounds ->
      ParenthesizedType
    | TypePath
    | UnitType
    | ReferenceType
    | ArrayType

ParenthesizedType -> `(` Type `)`
```

## Supported types

| Type | Meaning |
| --- | --- |
| `i32`, `isize` | Distinct signed 32-bit integers |
| `u32`, `usize` | Distinct unsigned 32-bit integers |
| `bool` | Boolean |
| Named struct; `Self` in its declaration or impl | Nominal named-field product |
| `[T; N]` | Fixed array with a restricted `usize` constant length |
| `&T`, `&mut T`, `&'a T`, `&'a mut T` | Shared or mutable reference |
| `()` | Function or control-flow result |
| [`Box<T>`, `Vec<T>`](types/heap.md) | Owned heap object or dynamic sequence |

Diverging expressions use the [never type](types/never.md) internally.

Struct types are nominal. References, arrays, `Box`, and `Vec` are structural. The four integer types remain distinct. Array lengths are compared by value, so `[i32; 4]` and `[i32; (4usize)]` are identical. Two reference types are identical if and only if their referent types and mutabilities match; lifetime arguments do not distinguish reference types and must satisfy the [lifetime validity guarantee](undefined-behavior.md#lifetime-validity).

## Inference

Type inference fills in omitted local binding types from their initializers. Function and item signatures are explicit. Each binding has one fixed type; later uses do not change it. Omitting explicit concrete type arguments in `Box` and `Vec` constructor calls is [undefined behavior](heap.md#builtin-signatures). A receiver's type must be known before field or method lookup.

### Integer literals

Integer literals use this limited inference rule, in order:

1. A suffix fixes the type.
2. Without a suffix, use the expected type if it is one of `i32`, `u32`, `isize`, or `usize`.
3. Otherwise, use `i32`.

Expected types come from the [contexts and propagation rules below](#coercion-sites-and-expected-types). Without such an expectation, another operand, a neighboring array element or branch, and later uses of a binding do not select the literal's type. Once chosen, an integer type does not change implicitly.

```rust,ignore
let n = 1;                   // i32: no expected type
let count: u32 = 1;          // u32: from the annotation
let sum: u32 = 1 + 2;        // undefined behavior: needs propagation through +
let neg: isize = -1;         // undefined behavior: needs propagation through unary -
let r: &u32 = &1;            // undefined behavior: needs propagation through &
let values: [u32; 2] = [1, 2]; // u32 elements; usize length
let bad: u32 = 1i32;         // compile error: a suffix fixes the type
let later: u32 = n;          // compile error: n is already i32
let mixed = 1 + 2u32;        // compile error: no expected type for 1
```

Array lengths and indices have expected type `usize`, so `[i32; 4]`, `[0; 4]`, and `data[0]` are valid without suffixes. Range checking follows type selection: an integer literal whose value falls outside the selected type's representable range exhibits [undefined behavior](undefined-behavior.md#integer-literal-range). The magnitude of the literal never causes a different integer type to be selected.

## Conversions and coercions

An explicit conversion uses `as`. A coercion is an automatic adjustment at one of the expression positions listed below. The organization follows the [Rust Reference](https://doc.rust-lang.org/reference/type-coercions.html), with only the types and operations supported by Rx.

### Explicit conversions

Integer-to-integer and bool-to-integer conversions require `as`. Integer casts preserve the 32-bit pattern; `false` becomes 0 and `true` becomes 1. These are the only supported [casts](expressions/operator-expr.md#casts). Conditions require expressions of type `bool`; integer values cannot be used as conditions and are not implicitly converted.

### Coercion sites and expected types

A **one-to-one coercion** checks one expression against a known target, called its *expected type*. A **least upper bound (LUB) coercion** finds a common target for several expressions. This distinction follows the [Rust Compiler Development Guide](https://rustc-dev-guide.rust-lang.org/hir-typeck/coercions.html). An expected integer type can select an unsuffixed literal's type before coercion; coercion itself does not convert integers.

These sites have a known expected type and use one-to-one coercion:

| Expression position | Expected type |
| --- | --- |
| Annotated `let` initializer | Annotation |
| Constant initializer | Declared constant type, subject to the restricted [constant forms](const_eval.md) |
| Assignment right operand | Destination |
| Function, method, or container constructor argument | Declared parameter; method receivers follow [receiver adjustments](expressions/method-call-expr.md#method-lookup) |
| Function tail or `return` operand | Declared result |
| Struct field initializer | Declared field |

The expected type propagates through parentheses, into block tail expressions, to both result branches of an `if` expression, and to all `break` value expressions targeting a `loop`. An expected array type `[T; N]` supplies `T` to each listed or repeated element and requires length `N`. Each of these result expressions is checked against that target; failure is a compile error except for the propagation exclusion below.

Propagation repeats only through the forms listed above. It does not pass through binary operations, unary operations, or borrow expressions (`&e` and `&mut e`), and does not revisit earlier binding initializers. If a program requires type propagation through an operator or borrow expression to determine an integer literal's type, its behavior is **undefined behavior**, rather than triggering a required compile error.

This excludes reliance on propagation, not the operators or borrows themselves. For example, `let n: i32 = 1 + 2;` works by defaulting, and `let r: &u32 = &1u32;` works because the suffix fixes the type. Incompatible types fixed by suffixes or existing bindings remain compile errors. Call arguments still get their declared expected types, even inside an operator or borrow.

```rust,ignore
// Expected type u32 propagates into block tails, if branches, and loop breaks:
let x: u32 = { 1 };                            // valid: 1 receives expected type u32
let y: u32 = if true { 1 } else { 2 };         // valid: 1 and 2 receive expected type u32
let z: u32 = loop { break 42; };               // valid: 42 receives expected type u32

// But expected type does NOT penetrate operators or borrows:
let a: u32 = 1 + 2;                            // undefined behavior (requires propagation through +)
let b: &u32 = &1;                              // undefined behavior (requires propagation through &)
let c: &u32 = &1u32;                           // valid: suffix fixes the literal type
```

Conditions require `bool`, array lengths and indices have expected type `usize`, and loop bodies and an `if` without `else` require unit results. Operators follow their [operand rules](expressions/operator-expr.md). These requirements do not enable additional conversions.

### Coercion types

Only the following adjustments are supported:

| Source | Target | Adjustment |
| --- | --- | --- |
| `T` | `T` | Keep the type; ordinary copy/move rules apply, except for mutable-reference reborrowing below |
| `&mut T` | `&T` | Borrow the same value through a shared reference |
| `&S` or `&mut S` | `&T` | Follow builtin dereferences from `S` to a `T` place, then borrow it |
| `&mut S` | `&mut T` | Follow builtin dereferences from `S` to a mutable `T` place, then borrow it mutably |
| Never | Any target | No value is produced on that path |

The builtin dereference steps are `&U` to `U`, `&mut U` to `U`, and `Box<U>` to `U`. They may be repeated until the target's referent type matches exactly. A mutable target requires an outer `&mut` and no shared reference along the path. The adjustment borrows the reached value without moving its contents. `Vec` has no builtin dereference.

```rust,ignore
let mut boxed = Box::<i32>::new(1);
let r: &i32 = &boxed;           // &Box<i32> to &i32
let m: &mut i32 = &mut boxed;   // &mut Box<i32> to &mut i32
```

Coercion does not borrow an owned value implicitly: `Box<T>` cannot coerce to `T` or `&T`; write `&boxed` for the latter. It does not convert stored array or container elements, change inner reference mutability, or produce slices. For example, `&mut &mut i32` cannot coerce to `&mut &i32`. Array *literal elements* may each coerce because they are separate expression sites.

### Mutable-reference reborrowing

> **Note**: Since we guarantee that lifetimes are valid, you do not need to know about reborrowing for this project. It is retained here solely for the completeness of the specification.

At a coercion site, an `&mut T` may be borrowed again as `&mut T`. This is a *reborrow*: after the new borrow ends, the original reference is usable again. An unannotated `let` binding instead moves the reference:

```rust,ignore
let mut x = 1;
let p = &mut x;
{
    let q: &mut i32 = p; // reborrow
    *q = 2;
}
*p = 3;
let moved = p;           // move
*moved = 4;
```

After the move, `p` cannot be used without reinitialization. Passing `p` as an argument to an `&mut i32` parameter reborrows it in the same way. Later uses do not turn a move into a reborrow.

### Least upper bound coercions

When no expected type fixes the target, LUB coercion finds a common type for:

- The result branches of an `if` with `else`.
- The elements of an array literal.
- The values of all `break` expressions targeting the same `loop`.

With an expected type, these same expressions use one-to-one coercions to that fixed target. Function returns and the function tail also share a fixed target: the declared result type. They never infer a different function signature.

For an unknown target, first determine each result's type independently, using `i32` for unsuffixed integers that have no expected integer type. The current LUB target is not an expected type for another result. Consider results in source order and ignore never results, which can fit any target. Start with the first remaining result's type as the current target `T`. For each next result of type `U`:

1. If `U` can coerce to `T`, keep `T`.
2. Otherwise, if `T` can coerce to `U` and every earlier result can be adjusted to `U`, use `U` as the new target and adjust those results too.
3. If not, the behavior is undefined ([undefined behavior](undefined-behavior.md#test-guarantees)). No search for a third common type is required.

If all results are never, the result type is never. Empty arrays follow the [zero-sized-data exclusion](undefined-behavior.md#zero-sized-data). A bare `break;` supplies `()`; breaks targeting nested loops do not participate.

Each row below is a separate example:

| Expression | Result |
| --- | --- |
| `let a = [&mut 1, &123];` | `[&i32; 2]`; the first element becomes shared |
| `let a = [&123, &mut 1];` | The same type; the second element becomes shared |
| `let a = [&mut 1u32, &123u32];` | `[&u32; 2]` |
| `let a = [&mut 1u32, &123];` | Undefined behavior: no expected type for `123`, so it is `i32` |
| `let a: [&u32; 2] = [&mut 1, &123];` | Undefined behavior: inferring `u32` would require propagation through both borrows |
| `let a: [&u32; 2] = [&mut 1u32, &123u32];` | Valid: the literals are already `u32`; the first reference coerces to shared |
| `let a = [&mut &1, &mut &mut 2];` | Undefined behavior: inner reference mutability differs |
| `let a: [&mut &i32; 2] = [&mut &1, &mut &mut 2];` | Compile error: the second element cannot meet the fixed expected type |

These rules determine the types of expressions; they do not change evaluation order or the separate method and operator adjustment rules.

## Recursive types

A by-value struct or array containment cycle has no finite size and is a compile error. References, `Box`, and `Vec` provide indirection, so recursive and mutually recursive structs through those types are supported.

```rust,ignore
struct Node {
    value: i32,
    children: Vec<Node>,
}
```

<details>
<summary>Recursive layout details</summary>

`struct Infinite { child: Infinite }` is invalid because the child is inline; an inline array also fails to break the cycle. Referencing an invalid cyclical type inside a container (such as `Box` or `Vec`) within another declaration does not make the cyclic type definition valid.

```rust,ignore
// Invalid: infinite size due to inline containment cycle
// struct BadNode { child: BadNode }
// struct BadArray { items: [BadArray; 1] }

// Valid: indirection breaks the layout cycle
struct GoodNode {
    child: Box<GoodNode>,
}
```

Finite layout does not imply constructibility. `struct Chain { next: Box<Chain> }` has finite layout but no terminating value because `Box` is non-nullable. This does not make its declaration invalid.

An implementation may check cycles only through inline struct and array dependencies. Container APIs, trait capabilities, and reclamation are defined separately in [Box and Vec](heap.md).

</details>
