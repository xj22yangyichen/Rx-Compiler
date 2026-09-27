# Array types

```grammar,types
ArrayType -> `[` Type `;` ConstValue `]`
```

An array type specifies an element type and a [ConstValue] length. The element type can be any supported concrete type. Length typing follows [constant contexts](../const_eval.md#typing-and-range), and equality of array types follows [type identity](../types.md#supported-types).

Zero lengths are syntactically permitted, but evaluating expressions that access zero-sized array data exhibits [undefined behavior](../undefined-behavior.md#zero-sized-data). See [types and inference](../types.md) and [constant contexts](../const_eval.md).

```rust,ignore
let buffer: [u32; 16];           // array of 16 u32 values
let matrix: [[i32; 4]; 4];       // 4x4 nested array of i32 values
```
