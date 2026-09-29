# 表达式与求值

```grammar,expressions
Expression -> ExpressionWithoutBlock | ExpressionWithBlock

ExpressionWithoutBlock ->
      LiteralExpression
    | PathExpression
    | OperatorExpression
    | GroupedExpression
    | ArrayExpression
    | IndexExpression
    | UnitExpression
    | StructExpression
    | CallExpression
    | MethodCallExpression
    | FieldExpression
    | ContinueExpression
    | BreakExpression
    | ReturnExpression

ExpressionWithBlock ->
      BlockExpression
    | LoopExpression
    | IfExpression
```

表达式可以产生整数、布尔值和单元值；解析名称和关联路径；构造 Box/Vec 值、数组和具名字段结构体；访问字段和索引元素；调用函数和方法；应用运算符；并通过块、if/while/loop 以及 return/break/continue 控制执行。括号用于对表达式分组。堆操作在 [Box 与 Vec](heap.md) 中规定。

<a id="evaluation-order"></a>
## 求值顺序

普通操作数在外层操作之前从左到右求值。这包括调用实参、数组元素、结构体字段初始化器、比较、算术运算和索引。结构体字段按源代码顺序求值，而不是按声明或布局顺序求值。

[赋值](expressions/operator-expr.md#evaluation-order)、[短路逻辑](expressions/operator-expr.md#comparison-and-logic)、[数组重复](expressions/array-expr.md#array-expressions)和控制流的专门规则规定哪些操作数会被求值以及求值顺序。

优化必须保留可观察效果，包括 I/O 和通过引用进行的写入。

<a id="places-and-values"></a>
## 位置与值

位置表示存储：变量、解引用结果、字段、按索引访问的数组或 `Vec` 元素，或带括号的位置。读取位置时，根据其类型复制或移动其中的值。赋值要求可变位置，而借用的结果是包含该位置地址的引用。

可变局部变量可以被赋值。解引用 `&mut T` 会产生可变位置，除非该解引用是通过共享引用到达的；解引用 `&T` 不会产生可变位置。

下面的代码块是相互独立的示例。不带 `mut` 声明的绑定可以保存 `&mut T` 并修改其目标，但不能被重定向到另一个目标：

```rust,ignore
let mut x = 1;
let mut y = 2;
let p = &mut x;
*p = 3;        // valid: changes x, not the binding p
p = &mut y;    // compile error: p is not a mutable binding
```

使绑定可变后，两种操作都允许：

```rust,ignore
let mut x = 1;
let mut y = 2;
let mut p = &mut x;
*p = 3;        // valid: changes x
p = &mut y;    // valid: redirects p to y
*p = 4;        // valid: changes y
```

共享引用不允许修改其目标，即使该目标声明为可变：

```rust,ignore
let mut x = 1;
let p = &x;
*p = 2;        // compile error: p gives shared access to x
```

此外，不能通过指向 `&mut T` 的共享引用恢复可变访问：

```rust,ignore
let mut x = 1;
let p = &mut x;
let shared = &p;
**shared = 2;  // compile error: reaching x crosses the shared reference
```

<a id="temporary-places"></a>
### 临时位置

`values` 这样的局部变量命名已有的存储。`make_array()` 这样的值表达式产生一个值，但不命名存储位置。当某个操作需要该值的位置时，该值会保存在临时存储中。这称为*临时值实体化*，适用于字段和索引的基表达式、显式借用的值以及需要借用的方法接收者。

每次外层表达式求值时，值表达式只求值一次。字段、元素、借用或方法调用使用该次求值结果；获取其地址不得再次对它求值。例如，索引 `make_array()[0]` 只调用一次 `make_array()`，该调用的任何副作用也只发生一次。

临时值无需命名的 `let mut` 绑定也可以被可变地使用。对于函数 `make_array() -> [i32; 2]`，以下操作有效：

```rust,ignore
make_array()[0] = 7;          // writes an element of the temporary array
Vec::<i32>::new().push(5);    // borrows the temporary vector as &mut self
let p = &mut 3;              // borrows a temporary integer
*p = 4;                     // modifies that integer
```

对于任何包含该字段的返回结构体，同一规则也允许 `make().field = value`。已有位置仍遵循各自的可变性规则：以不可变局部变量或共享引用为基表达式不会为它创建可变副本。赋值仍要求位置表达式；`1 = 2` 和 `1 += 2` 无效。块按照普通复制/移动语义产生值。

<details>
<summary>临时值生命周期（可选背景知识）</summary>

临时值通常在包含它的语句结束前保持有效。某些 `let` 初始化器会将其存储期延长到外层块结束。在下面的示例中，初始化器中的显式借用延长了返回数组的生命周期：

```rust,ignore
fn make_array() -> [i32; 2] {
    println_i32(1);
    [10, 20]
}

fn main() {
    let r = &mut make_array()[0];
    *r = 99;
    println_i32(*r);
}
```

这会分别在两行输出 `1` 和 `99`。调用执行一次，其返回数组占用临时存储，而 `r` 引用该数组的第一个元素。该数组保持有效直到 `main` 的块结束。

从普通函数或方法调用返回引用本身不会延长实参临时值的生命周期。例如，使用上面的 `make_array` 函数：

```rust,ignore
fn first(values: &mut [i32; 2]) -> &mut i32 {
    &mut values[0]
}

fn example() {
    let r = first(&mut make_array()); // the temporary array ends at this semicolon
    *r = 99; // excluded lifetime violation: r would refer to expired storage
}
```

最后一个示例不在[生命周期有效性保证](undefined-behavior.md#lifetime-validity)范围内；编译器无需诊断它。符合规范的实现可以将临时存储保留到函数返回，也可以在保留可观察行为和有效引用的前提下将其优化掉，具体见[引用](references.md#storage-and-implementation)。保留存储不会使原本无效的源代码引用变得有效。

</details>

<a id="precedence"></a>
## 优先级

以下分组按优先级从高到低排列。同一行中的运算符共享优先级。

| Group | Associativity |
| --- | --- |
| 路径、字面量、分组 | 基本形式 |
| 字段和方法访问、调用、索引 | 后缀 |
| 一元 `-`、`!`、`*`、`&`、`&mut` | 前缀 |
| `as` | 左结合 |
| `*`、`/`、`%` | 左结合 |
| `+`、`-` | 左结合 |
| `<<`、`>>` | 左结合 |
| `&` | 左结合 |
| `^` | 左结合 |
| <code>&#124;</code> | 左结合 |
| `==`、`!=`、`<`、`<=`、`>`、`>=` | 不加括号不能链接 |
| `&&` | 左结合 |
| <code>&#124;&#124;</code> | 左结合 |
| `=`、`+=`、`-=`、`*=`、`/=`、`%=`、`&=`、`^=`、<code>&#124;=</code>、`<<=`、`>>=` | 右结合 |
| `return`、带值的 `break` | 消耗后续表达式 |

块/语句消歧还遵循[语句](statements.md#statement-boundary)中的规则。
