# Builtin traits

The compiler recognizes `Copy`, `Clone`, `PartialEq`, and `Eq`. Structs implement them by specifying [derive attributes](traits-and-attributes.md#attribute-grammar). The only builtin trait operations are `.clone()`, `==`, and `!=`; ordering and arithmetic operations are governed by [Operators](expressions/operator-expr.md).

## Copy

**Functionality.** Reading a place copies a `Copy` value and moves any other value. This applies to initialization, assignment, by-value arguments and receivers, returns, and field reads. A move makes its source unavailable until reinitialized; it never creates an implicit alias. `Copy` is a marker trait and introduces no methods.

Copies are semantically independent. Copying a shared reference copies the reference, not its target. A machine-level address-passing convention must preserve by-value semantics. Moves may reuse storage when no observable behavior changes.

Tests exclude reading uninitialized values, use after move, double move, invalid partial move, and moving non-`Copy` values from borrowed contents or array indices. The compiler is not required to diagnose these ownership violations, but valid moves across branches, through fields, and during early returns must be correctly supported.

**Requirements.** Deriving `Copy` requires every field of the struct to implement `Copy`, and `Clone` must also be explicitly derived on the struct. Deriving `Copy` must be explicitly requested even when a bitwise copy could implement `Clone`.

**Example.** Copy a small value while keeping the original usable.

```rust,ignore
#[derive(Clone, Copy)]
struct Point { x: i32, y: i32 }

fn main() {
    let original = Point { x: 1, y: 2 };
    let mut copied = original;
    copied.x = 9;
    print_i32(original.x); // 1
}
```

## Clone

**Functionality.** `Clone` provides `fn clone(&self) -> Self`. A derived `Clone` implementation recursively clones all fields and array elements to produce an independent value. Scalars and shared references are copied; the target of a shared reference is not cloned. Cloning semantics for owned heap containers are defined in [Container cloning](heap.md#clone-and-equality).

An explicit `.clone()` uses [method lookup](expressions/method-call-expr.md#method-lookup). If multiple candidate methods match the same candidate receiver type (such as an inherent method competing with a derived `clone` method), the method call results in undefined behavior.

**Requirements.** Deriving `Clone` requires every field to be `Clone`. Mutable references are not `Clone`.

**Example.** Duplicate owned heap data so that each value can change independently.

```rust,ignore
#[derive(Clone)]
struct Buffer { value: Box<i32> }

fn main() {
    let original = Buffer { value: Box::<i32>::new(7) };
    let mut cloned = original.clone();
    *cloned.value = 9;
    print_i32(*original.value); // 7
}
```

## PartialEq

**Functionality.** `PartialEq` enables `==` and `!=`, which borrow their operands. Scalar values are compared by value; arrays are compared element-wise; derived structs compare each corresponding field; and references compare their referents rather than their pointer addresses. Padding bytes are ignored, so a whole-object `memcmp` is generally invalid. `Box` and `Vec` follow [container equality](heap.md#clone-and-equality).

**Requirements.** Deriving `PartialEq` requires every field to be `PartialEq`. Both operands must share the exact same source type implementing `PartialEq`; the equality operators do not perform implicit type coercion and do not infer an unsuffixed integer literal's type from the opposing operand. Thus `1 == 1i32` is valid, while `1 == 1u32` is undefined behavior under the [cross-type equality guarantee](undefined-behavior.md#cross-type-equality).

**Example.** Compare records without consuming them.

```rust,ignore
#[derive(PartialEq)]
struct Reading { value: i32 }

fn main() {
    let first = Reading { value: 7 };
    let second = Reading { value: 7 };
    let same = first == second; // true
    let different = first != second; // false; both remain usable
}
```

## Eq

**Functionality.** `Eq` marks equality as an equivalence relation: reflexive, symmetric, and transitive. It introduces no new operations; equality comparisons continue to use `PartialEq`.

**Requirements.** Deriving `Eq` requires every field to be `Eq` and `PartialEq` to be explicitly requested for the struct.

**Example.** Declare that record identifiers have equivalence-based equality.

```rust,ignore
#[derive(PartialEq, Eq)]
struct Key { id: i32 }

fn same_key(left: &Key, right: &Key) -> bool {
    left == right // compares id fields
}
```

## Trait capability summary

| Type | `Copy` | `Clone` | `PartialEq` / `Eq` with the same source type |
| --- | --- | --- | --- |
| Integers, `bool` | Yes | Yes | Both |
| `&T` | Yes | Yes | Requires the corresponding capability of `T` |
| `&mut T` | No | No | Requires the corresponding capability of `T` |
| `[T; N]` | If `T` is `Copy` | If `T` is `Clone` | Requires the corresponding capability of `T` |
| Struct | Valid explicit derive | Valid explicit derive | Valid explicit derive |
| `Box<T>`, `Vec<T>` | No | If `T` is `Clone` | Requires the corresponding capability of `T` |

Zero-sized data follows the [zero-sized-data guarantee](undefined-behavior.md#zero-sized-data). Trait capability verification must terminate even when evaluating recursive type definitions, while still enforcing each field's trait requirements. For example, `struct Node { children: Vec<Node> }` may derive `Clone`, `PartialEq`, and `Eq`, provided all other fields satisfy the corresponding trait requirements.
