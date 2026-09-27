# Box and Vec types

`Box` and `Vec` types are represented by a [TypePath] with [GenericArgs]. Their syntax follows the
[type-path rules](../paths.md#paths-in-types); name resolution resolves the
container type name, which requires exactly one concrete type argument.

## Box types

`Box<T>` is an owning container for one value of type `T`. A `Box` has a
fixed-size representation referring to separately allocated heap storage for its
contents. Its constructor and dereference operations are described in
[Builtin signatures](../heap.md#builtin-signatures) and [Box access and
moves](../heap.md#box-access-and-moves).

## Vec types

`Vec<T>` is an owning sequence of `T` values with a runtime length. The type
argument determines the element type. The sequence's elements occupy contiguous heap
storage, while the `Vec` value itself has a fixed-size representation. Its methods and
indexing operation are specified in [Builtin signatures](../heap.md#builtin-signatures)
and [Vec operations](../heap.md#vec-operations).

## Type arguments and composition

The type argument may be any supported concrete type, including another container, array, reference, or named struct. `Box` and `Vec` may occur anywhere [Type] is accepted. The [zero-sized-data rules](../undefined-behavior.md#zero-sized-data) govern element and referent types.

<details>
<summary>Composition examples</summary>

| Type spelling | Meaning |
| --- | --- |
| `Box<i32>` or `Box::<i32>` | One owned integer |
| `Vec<u32>` | A sequence of unsigned integers |
| `Box<[i32; 4]>` | One owned four-element array |
| `Vec<Box<i32>>` | A sequence of owned integer boxes |
| `Box<Vec<i32>>` | One owned vector |
| `Vec<&i32>` or `Vec<&'a i32>` | A sequence of shared references |
| `Vec<View<'a>>` | A sequence of structs with a lifetime argument |
| `&mut Vec<Box<i32>>` | A mutable reference to a vector of boxes |
| `[Vec<i32>; 2]` | A two-element array of vectors |
| `Vec<Vec<i32,>,>` | Nested vectors with trailing type-argument commas |

```rust,ignore
struct Storage {
    values: Vec<Box<i32>>,
    counts: Box<[i32; 4]>,
}

fn append(values: &mut Vec<Box<i32>>, value: Box<i32>) {
    values.push(value);
}
```

</details>

The [recursive-type rules](../types.md#recursive-types) define how container indirection permits finite recursive layouts. Lifetime arguments follow the [lifetime rules](../references.md#lifetime-validity). The container type of an unannotated local variable is inferred from its initializer, but [never from subsequent statements](../types.md#inference).

Container values are created by the [builtin constructors](../heap.md#builtin-signatures).
Path separators, turbofish syntax, and trailing argument commas follow
[Paths](../paths.md); combined closing angle brackets (`>>`) follow
[contextual punctuation](../grammar.md#contextual-punctuation).
