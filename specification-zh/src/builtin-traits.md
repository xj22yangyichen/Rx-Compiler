<a id="builtin-traits"></a>
# 内建 Trait

编译器识别 `Copy`、`Clone`、`PartialEq` 和 `Eq`。结构体通过指定[派生属性](traits-and-attributes.md#attribute-grammar)实现它们。唯一的内建 Trait 操作是 `.clone()`、`==` 和 `!=`；排序与算术操作由[运算符](expressions/operator-expr.md)规定。

<a id="copy"></a>
## Copy

**功能。** 读取位置时，`Copy` 值会被复制，其他值则会被移动。这适用于初始化、赋值、按值传递的实参和接收者、返回以及字段读取。移动使源位置在重新初始化前不可用；它绝不会创建隐式别名。`Copy` 是标记 Trait，不引入任何方法。

各副本在语义上相互独立。复制共享引用只复制引用，不复制其目标。机器级的地址传递约定必须保留按值语义。在不改变任何可观察行为时，移动可以复用存储。

测试排除读取未初始化值、移动后使用、重复移动、无效的部分移动，以及从借用的内容或数组索引处移动非 `Copy` 值。编译器不必诊断这些所有权违规，但必须正确支持跨分支、通过字段以及提前返回过程中的有效移动。

**要求。** 派生 `Copy` 要求结构体的每个字段都实现 `Copy`，且结构体还必须显式派生 `Clone`。即使按位复制足以实现 `Clone`，也必须显式请求派生 `Copy`。

**示例。** 复制一个小值，同时使原值仍可使用。

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

<a id="clone"></a>
## Clone

**功能。** `Clone` 提供 `fn clone(&self) -> Self`。派生的 `Clone` 实现递归克隆所有字段和数组元素，以产生独立的值。标量和共享引用会被复制；共享引用的目标不会被克隆。拥有所有权的堆容器的克隆语义定义于[容器克隆](heap.md#clone-and-equality)。

显式的 `.clone()` 使用[方法查找](expressions/method-call-expr.md#method-lookup)。如果多个候选方法匹配同一个候选接收者类型（例如固有方法与派生的 `clone` 方法竞争），则该方法调用导致未定义行为。

**要求。** 派生 `Clone` 要求每个字段都为 `Clone`。可变引用不是 `Clone`。

**示例。** 复制拥有所有权的堆数据，使各个值能够独立改变。

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

<a id="partialeq"></a>
## PartialEq

**功能。** `PartialEq` 启用 `==` 和 `!=`，它们会借用操作数。标量按值比较；数组逐元素比较；派生的结构体比较每个对应字段；引用比较其所引用的值，而非指针地址。填充字节被忽略，因此对整个对象执行 `memcmp` 通常无效。`Box` 和 `Vec` 遵循[容器相等性](heap.md#clone-and-equality)。

**要求。** 派生 `PartialEq` 要求每个字段都为 `PartialEq`。两个操作数必须具有完全相同且实现 `PartialEq` 的源类型；相等运算符不执行隐式类型强制转换，也不根据另一操作数推断无后缀整数字面量的类型。因此 `1 == 1i32` 有效，而根据[跨类型相等性保证](undefined-behavior.md#cross-type-equality)，`1 == 1u32` 属于未定义行为。

**示例。** 比较记录而不消耗它们。

```rust,ignore
#[derive(PartialEq)]
struct Reading { value: i32 }

fn main() {
    let first = Reading { value: 7 };
    let second = Reading { value: 7 };
    let same = first == second; // true
    let different = first != second; // false；二者仍可使用
}
```

<a id="eq"></a>
## Eq

**功能。** `Eq` 标记相等性为等价关系：具有自反性、对称性和传递性。它不引入新操作；相等性比较继续使用 `PartialEq`。

**要求。** 派生 `Eq` 要求每个字段都为 `Eq`，且为结构体显式请求 `PartialEq`。

**示例。** 声明记录标识符的相等性为等价关系。

```rust,ignore
#[derive(PartialEq, Eq)]
struct Key { id: i32 }

fn same_key(left: &Key, right: &Key) -> bool {
    left == right // 比较 id 字段
}
```

<a id="trait-capability-summary"></a>
## Trait 能力汇总

| 类型 | `Copy` | `Clone` | 相同源类型之间的 `PartialEq` / `Eq` |
| --- | --- | --- | --- |
| 整数、`bool` | 是 | 是 | 二者皆有 |
| `&T` | 是 | 是 | 要求 `T` 具有相应能力 |
| `&mut T` | 否 | 否 | 要求 `T` 具有相应能力 |
| `[T; N]` | 当 `T` 为 `Copy` 时 | 当 `T` 为 `Clone` 时 | 要求 `T` 具有相应能力 |
| 结构体 | 有效的显式派生 | 有效的显式派生 | 有效的显式派生 |
| `Box<T>`、`Vec<T>` | 否 | 当 `T` 为 `Clone` 时 | 要求 `T` 具有相应能力 |

零大小数据遵循[零大小数据保证](undefined-behavior.md#zero-sized-data)。即使在求值递归类型定义时，Trait 能力验证也必须终止，同时仍要落实每个字段的 Trait 要求。例如，只要所有其他字段满足相应 Trait 要求，`struct Node { children: Vec<Node> }` 就可以派生 `Clone`、`PartialEq` 和 `Eq`。
