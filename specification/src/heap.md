# Box and Vec

The [Box and Vec types](types/heap.md) chapter defines container representations and type rules. This chapter defines their operations, reference semantics, and allocation lifecycles. The [capability table](builtin-traits.md#trait-capability-summary) defines `Copy`, `Clone`, and equality support.

## Builtin signatures

In these signatures, `T` stands for the container's concrete type argument.
`&self` takes a shared reference to the container; `&mut self` takes a mutable
reference. `new` is an associated function with no receiver. The other entries are methods and
follow the ordinary [method-call rules](expressions/method-call-expr.md).
None of the operations in this table requires a builtin trait capability of
`T`; arguments follow the ordinary [call and coercion rules](expressions/call-expr.md).

| Container | Signature | Behavior |
| --- | --- | --- |
| `Box<T>` | `fn new(value: T) -> Box<T>` | Moves or copies `value` into new owned storage. |
| `Vec<T>` | `fn new() -> Vec<T>` | Creates an empty vector. |
| `Vec<T>` | `fn len(&self) -> usize` | Returns the number of initialized elements. |
| `Vec<T>` | `fn is_empty(&self) -> bool` | Returns whether the length is zero. |
| `Vec<T>` | `fn push(&mut self, value: T) -> ()` | Appends one element by copy or move. |
| `Vec<T>` | `fn remove(&mut self, index: usize) -> T` | Moves out and returns the indexed element, shifting later elements left. |

In valid programs, constructors have explicit concrete type arguments:

```rust,ignore
let boxed = Box::<i32>::new(7);
let mut values = Vec::<i32>::new();
```

All container constructors--including nested ones--must provide explicit concrete type arguments.
Omitting this argument, as in `Box::new(7)` or `Vec::new()`, is undefined
behavior under the [test guarantees](undefined-behavior.md#test-guarantees).
Lifetime arguments inside `T` follow the [lifetime rules](references.md#lifetime-validity);
type-position `_` is unsupported.

## Box access and moves

For `b: Box<T>`, the builtin dereference expression `*b` is a `T` place.
It is mutable through a mutable owner or mutable reference and
read-only through shared access. Normal field, index, method, and borrow
operations apply after supported `Box` and reference dereferences.

Dereferencing an owned `Box` does not insert a shared borrow. An immutable owner
prevents replacing its contents, but an `&mut U` stored in those contents can
still provide mutable access to `U`. Access through `&Box<T>` crosses a shared
reference and cannot provide this mutable access.

Reading `Copy` contents copies them. Reading non-`Copy` contents from an owned `Box` moves them; moving through a borrowed `Box` is excluded by the ownership guarantee. An allocation whose contents were moved may remain allocated until program end.

## Vec operations

For `v: Vec<T>` and `index: usize`, the builtin index expression `v[index]`
is a `T` place.

Indexing and removal require `index < len`; out-of-bounds execution is excluded with no required check or panic. `remove` preserves order and supports non-`Copy`, non-`Clone` elements. Reading a `Copy` indexed element copies it; directly moving a non-`Copy` indexed element is undefined behavior (excluded by test guarantees), so elements must be extracted using `remove`. Assignment to a mutable indexed place is supported.

```rust,ignore
let mut v = Vec::<i32>::new();
v.push(10);
v.push(20);
let first = v[0];               // copying an i32 element
let removed = v.remove(0);      // removing element at index 0
```

`Vec` itself is not dereferenceable and never coerces to an element or slice
reference.

### Indexing and mutable access

`Vec` indexing implicitly borrows the vector. Shared reads and
[`&mut T` to `&T` coercions](types.md#coercion-types) require only shared access.
Assignment, compound assignment, mutable borrowing or
reborrowing, and an `&mut self` receiver require mutable access to the vector
at that indexing step.

The requirement persists through fields, further indexing, `Box` dereferences,
and mutable references stored in the element. An `&mut T` inside a vector does
not make an immutable vector mutable:

```rust,ignore
fn shared_access(values: Vec<&mut i32>) {
    let r: &i32 = values[0]; // valid: &mut i32 coerces to &i32
}

fn immutable_vector(values: Vec<&mut i32>) {
    *values[0] = 2; // compile error: add mut to the values parameter
}

fn mutable_vector(mut values: Vec<&mut i32>) {
    *values[0] = 2; // valid: the vector can be borrowed mutably
}

fn mutable_reference(values: &mut Vec<&mut i32>) {
    *values[0] = 2; // valid: the reference supplies mutable access
}
```

Mutable reborrows and method receivers require the same mutable access even without an
explicit assignment to the element. Both functions below require `mut values`:

```rust,ignore
fn mutable_reborrow(values: Vec<&mut i32>) {
    let r: &mut i32 = values[0]; // compile error
}

fn nested_method(values: Vec<Vec<i32>>) {
    values[0].push(2); // compile error: push needs a mutable receiver
}
```

Fixed-array indexing and owned `Box` dereferencing do not insert a container
borrow. A mutable reference stored there can therefore remain usable through
an immutable owner. The position of the mutable reference matters:

```rust,ignore
fn array_reference(values: [&mut i32; 1]) {
    *values[0] = 2; // valid, unlike immutable_vector above
}

fn boxed_reference(value: Box<&mut i32>) {
    **value = 2; // valid: dereferencing the stored mutable reference
}

fn boxed_vector(values: Box<Vec<&mut i32>>) {
    *values[0] = 2; // compile error: the vector has an immutable owner
}

fn boxed_vector_reference(values: Box<&mut Vec<&mut i32>>) {
    *values[0] = 2; // valid: mutable access is obtained before Vec indexing
}
```

## Clone and equality

When `T` is `Clone`, the containers support these builtin `Clone` methods:

| Type | Method signature | Result |
| --- | --- | --- |
| `Box<T>` | `fn clone(&self) -> Box<T>` | Clones the stored `T` into independently owned storage. |
| `Vec<T>` | `fn clone(&self) -> Vec<T>` | Clones elements in order into an independent vector of equal length. |

Owned nested containers clone recursively; shared references are copied without
cloning their targets. The `Clone` capability is specified in [Builtin traits](builtin-traits.md#clone).
Dot calls remain subject to the [competing-method exclusion](expressions/method-call-expr.md#method-lookup): for example, use `Box::<i32>::clone(&boxed)` to select the box's clone explicitly.

`Box` equality compares stored values. `Vec` equality compares length and corresponding elements, ignoring allocation address, capacity, and padding. These operations compose with struct derives and finite values of recursive types.

## Storage and references

A `Vec` stores exactly its initialized sequence; spare capacity is not initialized data. Capacity and growth are unobservable implementation choices. Reallocation preserves values and order but invalidates references into the old buffer. `remove` invalidates references to removed or shifted elements.

Tests obey Rust's borrowing rules, including supported two-phase receiver borrowing such as `v.push(v.len())`. They never use an element reference across a conflicting mutable container operation, regardless of spare capacity. No borrow checker is required.

Zero-sized heap objects are undefined behavior under the [zero-sized-data guarantee](undefined-behavior.md#zero-sized-data). An empty `Vec<T>` for nonzero-sized `T` is valid and need not have a dereferenceable buffer. All allocations in valid tests fit within the target's 32-bit `usize` address space, and test cases do not depend on allocation failure.

## Heap cleanup

Student-generated code does not need to free heap allocations. Heap leaks are
allowed: storage from `Box` and `Vec` may remain allocated after scope exit,
overwrite, move-out, return, jumps, or `Vec` buffer replacement. No per-value
destruction, deallocation call, or final heap-cleanup code is required.

This does not change source ownership or reference validity. Moves still
transfer ownership, clones still create independent owned contents, and a
reference invalidated by reallocation or removal remains invalid even if the
old bytes are still present.

An implementation may reclaim or reuse storage earlier when it preserves
program behavior and valid references. Tests fit the cumulative heap budget
defined by the [backend resource guarantees](backend.md#resource-guarantees).
