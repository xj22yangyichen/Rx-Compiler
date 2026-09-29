<a id="builtin-io-and-program-entry"></a>
# 内建 I/O 与程序入口

<a id="program-entry"></a>
## 程序入口

每个可执行程序都有一个顶层入口函数：

```rust,ignore
fn main() -> () {
    println_i32(42);
}
```

可以省略返回类型标注。`main` 函数的参数列表为空，且没有泛型参数。

程序入口调用这个源函数，并在初次调用完成后返回状态 0。推荐实现为源函数分配唯一的经名字改编的内部符号，并将 C ABI 包装函数导出为 `main`。包装函数调用源函数并返回有符号 32 位值 0。等价的降级方式必须保留这些行为，具体见[后端契约](../backend.md#symbols-entry-and-runtime)。

程序使用以下 I/O 函数打印答案。

<a id="io-functions"></a>
## I/O 函数

以下接口记法描述内建签名：

```rust,ignore
fn get_i32() -> i32;
fn print_i32(value: i32) -> ();
fn println_i32(value: i32) -> ();
```

`get_i32` 从标准输入读取一个有符号十进制 `i32`。保证测试输入包含足够多的有效且范围内的整数，以 ASCII 空白字符分隔；排除格式错误的输入和输入提前结束的情况。`print_i32` 输出有符号十进制表示，不附加其他字符。`println_i32` 输出相同表示，后接换行字节（`'\n'`、`0x0A`）。

实参是普通的 `i32` 值。其他整数类型的值需要显式类型转换（例如 `print_i32(x as i32)`）。单元结果可以作为表达式语句丢弃；它们不允许普通的零大小绑定。

这三个函数构成源语言可见的 I/O 接口。学生可以使用提供的运行时，也可以按后端契约实现兼容的运行时。

<a id="other-builtin-facilities"></a>
## 其他内建设施

[内建 Trait](../builtin-traits.md)、[数组方法](../expressions/array-expr.md#array-methods)以及 [Box 与 Vec 操作](../heap.md)在各自章节中定义。[运行时契约](../backend.md#heap-allocation-interface)定义生成代码的分配接口。
