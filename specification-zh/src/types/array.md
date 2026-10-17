<a id="array-types"></a>
# 数组类型

```grammar,types
ArrayType -> `[` Type `;` ConstValue `]`
```

数组类型指定元素类型和 [ConstValue] 长度。元素类型可以是任何受支持的具体类型。长度的类型确定遵循[常量上下文](../const_eval.md#typing-and-range)，数组类型的相等性遵循[类型同一性](../types.md#supported-types)。

语法上允许零长度，但对访问零大小数组数据的表达式求值具有[未定义行为](../undefined-behavior.md#zero-sized-data)。参见[类型与推断](../types.md)以及[常量上下文](../const_eval.md)。

```rust,ignore
let buffer: [u32; 16] = [0u32; 16];      // 由 16 个 u32 值组成的数组
let matrix: [[i32; 4]; 4] = [[0; 4]; 4]; // 由 i32 值组成的 4x4 嵌套数组
```
