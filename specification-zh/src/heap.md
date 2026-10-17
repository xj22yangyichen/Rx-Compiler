<a id="box-and-vec"></a>
# Box 与 Vec

[Box 与 Vec 类型](types/heap.md)一章定义容器的表示和类型规则。本章定义它们的操作、引用语义和分配生命周期。[能力表](builtin-traits.md#trait-capability-summary)定义 `Copy`、`Clone` 和相等性支持。

<a id="builtin-signatures"></a>
## 内建签名

在这些签名中，`T` 表示容器的具体类型实参。`&self` 接受容器的共享引用；`&mut self` 接受可变引用。`new` 是没有接收者的关联函数。其他条目是方法，遵循普通的[方法调用规则](expressions/method-call-expr.md)。此表中的操作均不要求 `T` 具备任何内建 Trait 能力；实参遵循普通的[调用与强制转换规则](expressions/call-expr.md)。

| 容器 | 签名 | 行为 |
| --- | --- | --- |
| `Box<T>` | `fn new(value: T) -> Box<T>` | 将 `value` 移动或复制到新拥有的存储中。 |
| `Vec<T>` | `fn new() -> Vec<T>` | 创建空向量。 |
| `Vec<T>` | `fn len(&self) -> usize` | 返回已初始化元素的数量。 |
| `Vec<T>` | `fn is_empty(&self) -> bool` | 返回长度是否为零。 |
| `Vec<T>` | `fn push(&mut self, value: T) -> ()` | 通过复制或移动追加一个元素。 |
| `Vec<T>` | `fn remove(&mut self, index: usize) -> T` | 移出并返回指定索引的元素，将后续元素左移。 |

在有效程序中，构造函数具有显式的具体类型实参：

```rust,ignore
let boxed = Box::<i32>::new(7);
let mut values = Vec::<i32>::new();
```

所有容器构造函数（包括嵌套的构造函数）都必须提供显式的具体类型实参。根据[测试保证](undefined-behavior.md#test-guarantees)，省略此实参（如 `Box::new(7)` 或 `Vec::new()`）属于未定义行为。`T` 内的生命周期实参遵循[生命周期规则](references.md#lifetime-validity)；不支持类型位置的 `_`。

<a id="box-access-and-moves"></a>
## Box 的访问与移动

对于 `b: Box<T>`，内建解引用表达式 `*b` 是一个 `T` 类型的位置。通过可变所有者或可变引用访问时它可变，通过共享访问时它只读。在受支持的 `Box` 和引用解引用之后，可以应用普通的字段、索引、方法和借用操作。

解引用拥有所有权的 `Box` 不会插入共享借用。不可变所有者会阻止替换其内容，但内容中存储的 `&mut U` 仍可提供对 `U` 的可变访问。通过 `&Box<T>` 访问会经过共享引用，因此不能提供这种可变访问。

读取 `Copy` 内容会复制它。读取拥有所有权的 `Box` 中的非 `Copy` 内容会移动它；所有权保证排除了通过借用的 `Box` 移动内容的情况。内容已被移出的分配可以一直保留到程序结束。

<a id="vec-operations"></a>
## Vec 操作

对于 `v: Vec<T>` 和 `index: usize`，内建索引表达式 `v[index]` 是一个 `T` 类型的位置。

索引和移除要求 `index < len`；测试排除越界执行，不要求检查或 panic。`remove` 保持顺序，并支持非 `Copy`、非 `Clone` 元素。读取索引处的 `Copy` 元素会复制它；直接移动索引处的非 `Copy` 元素属于未定义行为（由测试保证排除），因此必须使用 `remove` 提取元素。支持对可变索引位置赋值。

```rust,ignore
let mut v = Vec::<i32>::new();
v.push(10);
v.push(20);
let first = v[0];               // 复制一个 i32 元素
let removed = v.remove(0);      // 移除索引 0 处的元素
```

`Vec` 本身不可解引用，也绝不会强制转换为元素引用或切片引用。

<a id="indexing-and-mutable-access"></a>
### 索引与可变访问

`Vec` 索引会隐式借用向量。共享读取以及 [`&mut T` 到 `&T` 的强制转换](types.md#coercion-types)只要求共享访问。赋值、复合赋值、可变借用或重借用以及 `&mut self` 接收者，都要求在该索引步骤具有对向量的可变访问。

这一要求会持续穿过字段、进一步的索引、`Box` 解引用以及元素中存储的可变引用。向量内的 `&mut T` 不会使不可变向量变为可变：

```rust,ignore
fn shared_access(values: Vec<&mut i32>) {
    let r: &i32 = values[0]; // 有效：&mut i32 强制转换为 &i32
}

fn immutable_vector(values: Vec<&mut i32>) {
    *values[0] = 2; // 编译错误：为 values 参数添加 mut
}

fn mutable_vector(mut values: Vec<&mut i32>) {
    *values[0] = 2; // 有效：可以可变借用向量
}

fn mutable_reference(values: &mut Vec<&mut i32>) {
    *values[0] = 2; // 有效：引用提供可变访问
}
```

即使没有显式地给元素赋值，可变重借用和方法接收者也要求相同的可变访问。下面两个函数都需要 `mut values`：

```rust,ignore
fn mutable_reborrow(values: Vec<&mut i32>) {
    let r: &mut i32 = values[0]; // 编译错误
}

fn nested_method(values: Vec<Vec<i32>>) {
    values[0].push(2); // 编译错误：push 需要可变接收者
}
```

定长数组索引和拥有所有权的 `Box` 解引用不会插入容器借用。因此，其中存储的可变引用仍可通过不可变所有者使用。可变引用所在的位置很重要：

```rust,ignore
fn array_reference(values: [&mut i32; 1]) {
    *values[0] = 2; // 有效，与上面的 immutable_vector 不同
}

fn boxed_reference(value: Box<&mut i32>) {
    **value = 2; // 有效：解引用存储的可变引用
}

fn boxed_vector(values: Box<Vec<&mut i32>>) {
    *values[0] = 2; // 编译错误：向量的所有者不可变
}

fn boxed_vector_reference(values: Box<&mut Vec<&mut i32>>) {
    *values[0] = 2; // 有效：在 Vec 索引之前获得了可变访问
}
```

<a id="clone-and-equality"></a>
## 克隆与相等性

当 `T` 为 `Clone` 时，容器支持以下内建 `Clone` 方法：

| 类型 | 方法签名 | 结果 |
| --- | --- | --- |
| `Box<T>` | `fn clone(&self) -> Box<T>` | 将存储的 `T` 克隆到独立拥有的存储中。 |
| `Vec<T>` | `fn clone(&self) -> Vec<T>` | 按顺序克隆元素，得到长度相同的独立向量。 |

拥有所有权的嵌套容器会递归克隆；共享引用只复制引用，不克隆其目标。`Clone` 能力在[内建 Trait](builtin-traits.md#clone) 中规定。如果多个方法匹配同一个候选接收者类型，点调用仍受[竞争方法排除规则](expressions/method-call-expr.md#method-lookup)约束。像 `Box::<i32>::clone(&boxed)` 这样的完全限定调用会直接选择容器操作，不经过方法查找。

`Box` 的相等性比较存储的值。`Vec` 的相等性比较长度及对应元素，忽略分配地址、容量和填充。这些操作可与结构体派生以及递归类型的有限值组合使用。

<a id="storage-and-references"></a>
## 存储与引用

`Vec` 恰好存储其已初始化的序列；空余容量不是已初始化数据。容量和增长方式是不可观察的实现选择。重新分配保留值及其顺序，但使指向旧缓冲区的引用失效。`remove` 使指向被移除或被移动元素的引用失效。

测试遵守 Rust 的借用规则，包括受支持的两阶段接收者借用，例如 `v.push(v.len())`。无论是否有空余容量，测试都不会跨越冲突的可变容器操作使用元素引用。不要求实现借用检查器。

根据[零大小数据保证](undefined-behavior.md#zero-sized-data)，零大小堆对象属于未定义行为。当 `T` 大小非零时，空 `Vec<T>` 有效，且不必具有可解引用的缓冲区。有效测试中的所有分配都位于目标的 32 位 `usize` 地址空间内，测试用例不依赖分配失败。

<a id="heap-cleanup"></a>
## 堆清理

学生生成的代码不需要释放堆分配。允许堆泄漏：`Box` 和 `Vec` 的存储可以在退出作用域、覆盖、移出、返回、跳转或替换 `Vec` 缓冲区后继续保持分配状态。不要求逐值析构、释放调用或最终堆清理代码。

这不会改变源语言的所有权或引用有效性。移动仍转移所有权，克隆仍创建独立拥有的内容；即使旧字节仍存在，因重新分配或移除而失效的引用也仍然无效。

只要保留程序行为和有效引用，实现可以更早回收或复用存储。测试满足[后端资源保证](backend.md#resource-guarantees)定义的累计堆预算。
