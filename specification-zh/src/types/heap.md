<a id="box-and-vec-types"></a>
# Box 与 Vec 类型

`Box` 和 `Vec` 类型由带 [GenericArgs] 的 [TypePath] 表示。它们的语法遵循[类型路径规则](../paths.md#paths-in-types)；名称解析会解析容器类型名称，该类型要求恰好一个具体类型实参。

<a id="box-types"></a>
## Box 类型

`Box<T>` 是拥有一个 `T` 类型值的所有权的容器。`Box` 具有固定大小的表示，指向为其内容单独分配的堆存储。其构造函数和解引用操作见[内建签名](../heap.md#builtin-signatures)以及 [Box 的访问与移动](../heap.md#box-access-and-moves)。

<a id="vec-types"></a>
## Vec 类型

`Vec<T>` 是拥有所有权、长度在运行时确定的 `T` 值序列。类型实参确定元素类型。序列的元素占据连续的堆存储，而 `Vec` 值本身具有固定大小的表示。其方法和索引操作在[内建签名](../heap.md#builtin-signatures)和 [Vec 操作](../heap.md#vec-operations)中规定。

<a id="type-arguments-and-composition"></a>
## 类型实参与组合

类型实参可以是任何受支持的具体类型，包括其他容器、数组、引用或具名结构体。任何接受 [Type] 的位置都可以出现 `Box` 和 `Vec`。[零大小数据规则](../undefined-behavior.md#zero-sized-data)约束元素类型和所引用的类型。

<details>
<summary>组合示例</summary>

| 类型写法 | 含义 |
| --- | --- |
| `Box<i32>` 或 `Box::<i32>` | 一个拥有所有权的整数 |
| `Vec<u32>` | 无符号整数序列 |
| `Box<[i32; 4]>` | 一个拥有所有权的四元素数组 |
| `Vec<Box<i32>>` | 拥有所有权的整数 box 序列 |
| `Box<Vec<i32>>` | 一个拥有所有权的向量 |
| `Vec<&i32>` 或 `Vec<&'a i32>` | 共享引用序列 |
| `Vec<View<'a>>` | 带生命周期实参的结构体序列 |
| `&mut Vec<Box<i32>>` | 指向 box 向量的可变引用 |
| `[Vec<i32>; 2]` | 由两个向量组成的数组 |
| `Vec<Vec<i32,>,>` | 类型实参带尾随逗号的嵌套向量 |

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

[递归类型规则](../types.md#recursive-types)定义容器提供的间接层如何使有限的递归布局成为可能。生命周期实参遵循[生命周期规则](../references.md#lifetime-validity)。未标注的局部变量的容器类型从其初始化器推断，但[绝不从后续语句推断](../types.md#inference)。

容器值由[内建构造函数](../heap.md#builtin-signatures)创建。路径分隔符、turbofish 语法和尾随实参逗号遵循[路径](../paths.md)；组合的右尖括号（`>>`）遵循[上下文标点符号](../grammar.md#contextual-punctuation)。
