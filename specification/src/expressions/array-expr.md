# Array and array index expressions

## Array expressions

r[expr.array.syntax]
```grammar,expressions
ArrayExpression -> `[` ArrayElements? `]`

ArrayElements ->
      Expression (`,` Expression)* `,`?
    | Expression `;` ConstValue
```

An array expression either lists its elements explicitly or repeats an element expression a constant number of times specified by a [ConstValue]. Explicitly listed elements are evaluated in the [ordinary evaluation order](../expressions.md#evaluation-order). Array repetition syntax (`[elem; len]`) evaluates its element expression once and repeats the resulting value; if the length is greater than 1, the element type must implement `Copy`, rather than merely `Clone`. The repetition length expression is evaluated according to the [constant-context rules](../const_eval.md#typing-and-range). Empty array literals and repetition expressions with a length of zero are syntactically valid, but any use of zero-sized array data is undefined behavior under the [zero-sized-data rules](../undefined-behavior.md#zero-sized-data).

```rust,ignore
let list = [1, 2, 3];           // explicit element list: [i32; 3]
let repeated = [0; 5];          // repetition: [i32; 5]
```

An expected array type supplies its element type to each listed or repeated element expression through [one-to-one coercions](../types.md#coercion-sites-and-expected-types). Without one, [LUB coercion](../types.md#least-upper-bound-coercions) finds the common element type.

## Array index expressions

r[expr.array.index.syntax]
```grammar,expressions
IndexExpression -> Expression `[` Expression `]`
```

### Index operand types

An index has expected type `usize`, so an unsuffixed literal index is inferred as
`usize`. Any index expression with an independently determined type must have type `usize`.

An index operand with the never type `!` exhibits
undefined behavior under the [test guarantees](../undefined-behavior.md#test-guarantees).
This applies to both arrays and `Vec`, such as `data[return]` and
`data[loop {}]`.

### Indexed places

Indexing repeatedly dereferences references and `Box` until it reaches the first
array or `Vec`, using the builtin steps under
[coercion types](../types.md#coercion-types).

The base expression precedes `[index]` and is evaluated exactly once. If it
evaluates to a value rather than an existing place (such as an array returned by a function call), that
value is materialized in temporary storage when needed to access its element. The temporary place's
mutability and storage duration follow the [temporary-place rules](../expressions.md#temporary-places).

Array indexing projects an element place without inserting a borrow of the
array. Its mutability follows the [place rules](../expressions.md#places-and-values).
`Vec` indexing follows the separate [mutable-access rule and examples](../heap.md#indexing-and-mutable-access).

Array indexing must stay within its fixed length in valid executions. Runtime bounds checks and panic handling are not required. Reading an element follows [copy/move semantics](../builtin-traits.md#copy), including the test exclusion for non-Copy moves through an array index. Vec access and removal follow the [Vec operation rules](../heap.md#vec-operations).

```rust,ignore
let mut data: [i32; 4] = [0; 4];
data[2] = 7;
println_i32(data[2]);
```

## Array methods

A fixed array's builtin `.len()` has receiver `&self` and returns its length as usize. When an array is passed by value, the entire array is copied (if [`Copy`](../builtin-traits.md#copy)) or moved as a single aggregate; array arguments never decay or convert to pointers.
