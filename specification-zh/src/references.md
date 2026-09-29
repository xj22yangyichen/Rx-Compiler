# References and storage duration

## Reference annotations

[Reference types](types/pointer.md) specify a referent type, mutability, and an optional lifetime. They may appear in struct fields, array element types, function parameter and return types, and generic container type arguments.

```rust,ignore
struct View<'a> { value: &'a i32 }

fn choose<'a>(a: &'a i32, b: &'a i32, first: bool) -> &'a i32 {
    if first { a } else { b }
}
```

## Lifetime validity

Declarations, annotations, arguments, bounds, and elision follow Rust 2021's [generic parameter](https://doc.rust-lang.org/reference/items/generics.html) and [lifetime elision](https://doc.rust-lang.org/reference/lifetime-elision.html) rules for supported constructs. Invalid lifetime usage is undefined behavior under the [lifetime validity guarantee](undefined-behavior.md#lifetime-validity) (i.e., tests are guaranteed to be lifetime-valid, and the compiler is not required to diagnose lifetime errors).

The compiler may [discard lifetime syntax after parsing](grammar.md#syntax-that-may-be-discarded-after-parsing); it need not implement lifetime checking or inference. Reference types, their mutability, and their referents remain part of the program's semantics.

Each elided lifetime in a function's parameter list is assigned a distinct lifetime parameter. If exactly one input lifetime exists (elided or explicit), that lifetime is assigned to all elided output lifetimes. If there are multiple input lifetimes but one belongs to a reference receiver (`&self` or `&mut self`), that receiver's lifetime is assigned to all elided output lifetimes. Any other function signature with elided output lifetimes is ambiguous and requires explicit annotations. The placeholder lifetime `'_` is accepted where permitted by Rust 2021 elision rules.

Correct annotations describe validity; they never extend the storage duration of a local, temporary, or heap object.

## Borrowing rules

Ordinary type and mutability checks are required: mutable borrows require mutable places, shared references cannot be reborrowed mutably, and writes through shared references must be rejected. Borrow dataflow analysis is not required because tests guarantee actual borrowing and ownership validity.

Valid programs may use multiple shared references, disjoint mutable references, and reborrows with shorter lifetimes. A parent mutable reference becomes usable again once its reborrow expires. Passing a mutable reference to an `&mut` parameter implicitly reborrows the referent.

```rust,ignore
let mut x = 10;
let r = &mut x;
{
    let reborrow: &mut i32 = r; // child reborrow
    *reborrow = 20;
}
*r = 30;                        // parent reference is usable again
```

References need no runtime lifetime markers, reference counts, or ownership checks. Optimizations must still respect aliasing and reborrowing; distinct reference values need not denote distinct addresses.

## Storage and implementation

Supported Rust 2021 local and temporary scopes define source validity. A local remains valid until its scope ends unless moved or otherwise invalidated. Shadowing creates a new binding without ending the old value's storage duration. Statement temporaries normally last through the statement, and eligible `let` initializers receive Rust's temporary-lifetime extension.

A conforming baseline may allocate fixed stack storage per call and retain it until return. Recursive calls require independent stack frames, whereas stack storage for variables within a loop body does not need to accumulate across iterations (i.e., slot storage may be reused across iterations). Implementations may promote values, reuse slots, or eliminate storage whenever observable behavior and valid references are preserved.

Retaining bytes does not make a dangling source reference valid. Constant promotion is optional, and tests do not return references to promoted literals or other program-lifetime temporaries.

<details>
<summary>Excluded reference executions</summary>

Tests never:

- Use a reference after its target expires, moves in an invalidating way, is freed, or is reallocated.
- Return and use a reference to a callee local or insufficiently long-lived temporary.
- Access overlapping storage through conflicting active shared and mutable borrows.
- Use a parent mutable reference while a conflicting child reborrow is active.
- Move non-`Copy` data from borrowed contents or reuse a moved mutable reference.
- Reference uninitialized, misaligned, incorrectly typed, or otherwise invalid data.

Shared references may alias, and disjoint mutable borrows remain valid. Container and array accesses performed using runtime indices must also satisfy these borrow exclusivity rules.

</details>

Container-specific invalidation follows the [Box and Vec storage rules](heap.md#storage-and-references). Omitting heap deallocation (leaking heap storage) does not extend source-level reference validity.
