<a id="array-and-array-index-expressions"></a>
# 数组与数组索引表达式

<a id="array-expressions"></a>
## 数组表达式

r[expr.array.syntax]
```grammar,expressions
ArrayExpression -> `[` ArrayElements? `]`

ArrayElements ->
      Expression (`,` Expression)* `,`?
    | Expression `;` ConstValue
```

数组表达式要么显式列出各个元素，要么将一个元素表达式重复由 [ConstValue] 指定的常量次数。显式列出的元素按[普通求值顺序](../expressions.md#evaluation-order)求值。数组重复语法（`[elem; len]`）对元素表达式求值一次，然后重复所得的值；如果长度大于 1，元素类型必须实现 `Copy`，而不只是 `Clone`。重复长度表达式按照[常量上下文规则](../const_eval.md#typing-and-range)求值。空数组字面量和长度为零的重复表达式在语法上是合法的，但根据[零大小数据规则](../undefined-behavior.md#zero-sized-data)，任何对零大小数组数据的使用都是未定义行为。

```rust,ignore
let list = [1, 2, 3];           // 显式元素列表：[i32; 3]
let repeated = [0; 5];          // 重复：[i32; 5]
```

预期数组类型通过[一对一强制转换](../types.md#coercion-sites-and-expected-types)，向每个显式列出或重复的元素表达式提供其元素类型。没有预期类型时，由 [LUB 强制转换](../types.md#least-upper-bound-coercions)确定公共元素类型。

<a id="array-index-expressions"></a>
## 数组索引表达式

r[expr.array.index.syntax]
```grammar,expressions
IndexExpression -> Expression `[` Expression `]`
```

<a id="index-operand-types"></a>
### 索引操作数类型

索引的预期类型为 `usize`，因此无后缀的字面量索引会被推断为 `usize`。任何类型独立确定的索引表达式都必须具有 `usize` 类型。

根据[测试保证](../undefined-behavior.md#test-guarantees)，具有 never 类型 `!` 的索引操作数会导致未定义行为。这同时适用于数组和 `Vec`，例如 `data[return]` 和 `data[loop {}]`。

<a id="indexed-places"></a>
### 索引得到的位置

索引操作使用[强制转换类型](../types.md#coercion-types)中列出的内建步骤，反复解引用引用和 `Box`，直到遇到第一个数组或 `Vec`。

基表达式位于 `[index]` 之前，并且恰好求值一次。如果它求值得到的是值而非已有位置（例如函数调用返回的数组），则在需要访问其元素时，将该值物化到临时存储中。临时位置的可变性和存储期遵循[临时位置规则](../expressions.md#temporary-places)。

数组索引投影出一个元素位置，不会插入对数组的借用。其可变性遵循[位置规则](../expressions.md#places-and-values)。`Vec` 索引遵循单独的[可变访问规则与示例](../heap.md#indexing-and-mutable-access)。

在有效执行中，数组索引必须处于其固定长度范围内。不要求运行时边界检查和 panic 处理。读取元素遵循[复制/移动语义](../builtin-traits.md#copy)，包括测试排除通过数组索引移动非 Copy 值的情况。Vec 的访问和移除遵循 [Vec 操作规则](../heap.md#vec-operations)。

```rust,ignore
let mut data: [i32; 4] = [0; 4];
data[2] = 7;
println_i32(data[2]);
```

<a id="array-methods"></a>
## 数组方法

固定长度数组的内建 `.len()` 的接收者为 `&self`，并以 usize 返回其长度。按值传递数组时，整个数组作为单个聚合值被复制（如果实现了 [`Copy`](../builtin-traits.md#copy)）或移动；数组实参绝不会退化或转换为指针。
