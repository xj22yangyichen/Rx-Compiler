<a id="backend"></a>
# 后端

<a id="target-and-toolchain"></a>
## 目标与工具链

唯一的目标是采用 ILP32 数据模型的小端 RISC-V RV32IM。外部 C 和 REIMU 接口使用基本整数 [RISC-V ELF psABI](https://riscv-non-isa.github.io/riscv-elf-psabi-doc/)，即定义分别编译的代码如何互操作的处理器专用应用程序二进制接口。内部调用约定由实现定义。

| 组件 | 必需配置 |
| --- | --- |
| Clang/LLVM | 版本 22 |
| 目标 | `riscv32-unknown-elf` |
| ISA 与 ABI | `-march=rv32im -mabi=ilp32` |
| REIMU | 待定 |
| 执行内存 | `--memory=256M` |
| 栈 | `--stack=1M` |

编译器输出 GNU 风格的文本汇编，必须同时被 Clang 集成汇编器和指定版本的 REIMU 接受。Clang 汇编过程或 REIMU 解析、链接、执行过程中的任何失败都构成契约违规。不得输出 Clang 接受但 REIMU 不支持的汇编语法。测试不要求特定的指令选择、栈帧结构、寄存器分配器、符号布局或汇编文本。

RISC-V 指令行为遵循课程指定版本的 RISC-V 非特权 ISA 规范。外部调用行为遵循课程指定版本的 RISC-V ELF psABI。这两个版本在本次课程期间保持不变。

<a id="llvm-ir-and-code-generation"></a>
## LLVM IR 与代码生成

学生必须使用 LLVM IR 作为中间表示，并能够输出有效的文本 LLVM IR（`.ll`），供 Clang/LLVM 22 在课程目标上接受。IR 必须保留 Rx 语义，并使用与 RV32IM/ILP32 一致的目标三元组和数据布局。

在前端开发期间，学生可以用 Clang 编译此 IR，并在 REIMU 中运行生成的汇编，以便在自己的后端就绪之前检查程序行为。完成的编译器必须使用学生自行实现的后端，从 LLVM IR 生成 RISC-V 汇编。Clang 或 LLVM 的代码生成功能可用于验证，但不能代替该后端。

Clang 可以编译 C 运行时，生成的程序也可以调用自定义辅助函数。运行时辅助函数不得通过解释源程序或 IR 来代替代码生成。课程教师可以使用其他工具生成参考结果和性能基线。

<details>
<summary>使用 LLVM IR 验证前端</summary>

给定前端生成的 `student.ll` 和课程的 `runtime-example.c`，将二者编译为文本汇编并一起执行：

```sh
clang --target=riscv32-unknown-elf -march=rv32im -mabi=ilp32 \
  -O0 -S student.ll -o frontend-check.s
clang --target=riscv32-unknown-elf -march=rv32im -mabi=ilp32 \
  -O2 -fno-builtin -S runtime-example.c -o runtime.s
reimu --file=frontend-check.s,runtime.s --memory=256M --stack=1M \
  --output=frontend-check.out
```

将 `frontend-check.out` 与预期的源程序行为比较，即可独立于学生后端测试前端。程序从标准输入读取；`--output` 将其输出与 REIMU 的状态消息分开。此检查应使用相同的 RV32 目标，包括 32 位地址以及 `usize`/`isize`。

</details>

<a id="data-layout"></a>
## 数据布局

源语言中的整数和地址均为 32 位。下表给出参考存储布局；只要源程序行为和外部接口仍然正确，学生实现可以采用其他内部布局。

| 类型 | 大小（字节） | 对齐（字节） | 表示 |
| --- | ---: | ---: | --- |
| `i32`, `u32` | 4 | 4 | 32 位补码整数或无符号整数 |
| `isize`, `usize` | 4 | 4 | 与地址宽度相同 |
| `bool` | 1 | 1 | `false = 0`, `true = 1` |
| `&T`, `&mut T` | 4 | 4 | 一个机器地址 |
| `()` | 0 | 1 | 无数据 |

除此之外，只要保留可观察的源程序行为和所有外部接口，内部寄存器以及编译器生成的中间值的表示均由实现定义。引用不携带运行时生命周期信息。单元返回值不使用返回寄存器。

<details>
<summary>参考聚合类型布局</summary>

课程实现使用以下便于实现的布局，但学生实现可以选择其他内部布局：

- `[T; N]` 具有 `T` 的对齐要求，大小为 `size(T) * N`；元素 `i` 起始于 `i * size(T)`。
- 结构体字段按声明顺序排列，各字段位于满足其对齐要求的第一个偏移处。
- 结构体的对齐要求是各字段对齐要求的最大值，大小向上取整至该对齐值的倍数。
- 填充字节的值未指定，复制值时无需保留。
- `&[T; N]` 是一个字，而非切片胖指针。

跨越所选运行时接口的代码必须在两侧使用相同的布局。

</details>

<a id="calling-conventions-and-value-semantics"></a>
## 调用约定与值语义

普通函数、方法、递归和私有汇编辅助函数可以使用任意一致的内部约定。为方便起见推荐使用 psABI，但仅在实际的外部边界（如机器级 `main`、课程 C 运行时和 REIMU libc）处强制要求使用。

psABI 的调用约定规定参数和返回值的位置、调用者可以预期在调用后保持不变的寄存器，以及栈的对齐方式。对于普通的 32 位整数或引用参数，前八个使用 `a0` 至 `a7`，其余参数使用栈；32 位标量结果使用 `a0`。被调用者保留 `s0` 至 `s11` 并恢复 `sp`。若调用者需要调用者保存寄存器（`a0` 至 `a7`、`t0` 至 `t6` 或 `ra`）中的值在调用后仍然存在，则由调用者负责保存并恢复该值。在这些外部边界，栈必须遵循 psABI 的 16 字节对齐规则。链接的规范涵盖聚合类型参数及其余规则。

机器级间接访问必须保留源语言的按值语义。例如，在 `fn f(mut x: [i32; 3])` 内修改参数不得改变调用者的值。当旧值不再可观察时，可以在移动后复用存储。

方法可以将接收者降级为显式参数。定长数组在源语言层面仍是按值传递的聚合类型，不会发生 C 风格的数组到指针退化。

<a id="symbols-entry-and-runtime"></a>
## 符号、入口与运行时

全局机器符号 `main` 必须提供行为等价于 `int main(void)` 的 C ABI 入口点。它调用源语言的[入口函数](undefined-behavior/builtin.md#program-entry)，并在该次调用完成后于 `a0` 中返回有符号 32 位值 0。它必须遵守 psABI，包括栈对齐和寄存器保留要求。

推荐的降级方式是为源语言 `main` 分配唯一的经名字改编的内部符号，并生成导出机器符号 `main` 的包装函数。源语言对 `main` 的调用（包括递归调用）以内部源函数为目标，使用其普通的返回单元值的调用约定。从该函数返回时继续执行调用者；从包装函数返回时程序正常结束。源语言签名保持为 `fn main() -> ()`。

例如，以下类 C 示意代码展示了这种分离：

```c
static void __rx_source_main(void) {
    /* 降级后的源语言 main 函数体。 */
}

int main(void) {
    __rx_source_main();
    return 0;
}
```

内部符号的拼写仅作示例。源函数和辅助函数的符号名称由实现定义，但必须唯一，且不得与机器入口或运行时符号冲突。源函数可以使用任何受支持的内部调用约定。只要同时保留普通的返回单元值的源语言调用行为和机器入口契约，就允许等价的降级方式；输出代码中不必保留独立的包装函数和内部函数。

源语言的 I/O 内建函数使用以下 C ABI：

```c
int32_t get_i32(void);
void print_i32(int32_t value);
void println_i32(int32_t value);
```

其输入和输出行为由[内建 I/O](undefined-behavior/builtin.md#io-functions) 定义。学生可以使用课程运行时，也可以提供兼容实现。每个运行时符号恰好链接一次。

<details>
<summary>参考 C 运行时</summary>

仓库包含以下完整的参考实现：

```c
{{#include runtime-example.c}}
```

局部的 `int32_t` 和 `uint32_t` typedef 避免了对目标 C 库头文件的依赖。`rx_int32_is_four_bytes` 和 `rx_uint32_is_four_bytes` typedef 是编译期大小检查：当对应整数类型占四个字节时，各自声明长度为 1 的数组类型，否则声明长度为负数的无效数组类型。它们不分配存储，也不生成运行时指令。

可以使用以下命令编译：

```sh
clang --target=riscv32-unknown-elf -march=rv32im -mabi=ilp32 \
  -O2 -fno-builtin -S runtime-example.c -o runtime.s
```

</details>

<a id="heap-allocation-interface"></a>
## 堆分配接口

使用课程运行时的生成代码通过以下固定的 C ABI 分配 `Box` 和 `Vec` 存储，该接口对源语言不可见：

```c
void *__rx_alloc(uint32_t size, uint32_t align);
```

`size` 和 `align` 均以字节计。有效调用的大小非零，对齐值为 1 或 4。函数返回至少 `size` 个满足请求对齐要求的可访问字节，并且与所有尚未复用的分配互不重叠。有效测试中的分配都会成功；失败行为未指定。

不要求提供 `__rx_dealloc`。自定义运行时可以改用私有分配器、调用 REIMU `malloc`，或内联等价行为。在所有情况下，编译器计算类型大小、对齐、元素步长和容器容量，而运行时管理无类型存储。有效的分配计算结果可由 `usize` 表示，因此不需要动态溢出检查和 panic 支持。

REIMU 还提供与 psABI 兼容的 `memcpy`、`memmove` 和 `memset`。仅对已知互不重叠的范围使用 `memcpy`；范围可能重叠时使用 `memmove`。

<details>
<summary>建议的编译器与运行时职责划分</summary>

| 操作 | 编译器职责 | 运行时职责 |
| --- | --- | --- |
| `Box` 构造 | 求值、计算布局、初始化并返回所有者 | 分配对齐的字节存储 |
| `Vec` 增长 | 计算容量、维护元数据并移动存活元素 | 分配新缓冲区；可选地提供辅助函数 |
| 按值操作 | 选择复制或移动并保留值语义 | 可选地复制原始字节 |
| 容器克隆 | 递归克隆具体的 `T` | 为结果分配存储 |
| 程序结束 | 不要求按类型析构 | 回收此次执行的堆 |

</details>

<a id="resource-guarantees"></a>
## 资源保证

测试在总执行内存为 256 MiB、栈为 1 MiB 的环境中运行。

<!--

64 MiB 的数值是为测试预留的余量，不是源语言可见的堆大小，也不保证单次分配该大小一定成功。代码、静态数据、栈和堆共享 256 MiB 的执行内存限制。不另设静态数据限制。

<details>
<summary>参考 Vec 增长与分配策略</summary>

参考实现使用以下基线确定测试规模：

1. 空 `Vec` 的长度和容量均为零，不分配缓冲区。
2. `push` 将容量从 0 增长到 4，之后每当容量用满时翻倍。
3. 增长时分配新缓冲区，按顺序移动元素，并将旧缓冲区保留到程序结束。
4. `remove` 不缩减容量。
5. 克隆空 `Vec` 不分配任何存储；克隆非空 `Vec` 时恰好分配足以容纳其当前长度的空间。
6. `Box<T>` 恰好分配 `size(T)` 字节。

容量仍然不可观察，学生实现可以使用其他布局或增长因子。它们仍必须在公布的内存限制内完成相同的测试。

</details> -->

<!-- *规范待定：内存限制、栈大小和累计堆预算将与课程测试套件一起最终确定。* -->

<a id="assembly-and-execution"></a>
## 汇编与执行

REIMU 接受文本汇编文件，并在内部完成汇编、链接和执行。按上述方式将选定的 C 运行时编译为 `runtime.s` 后，使用以下命令运行学生后端的 `student.s`：

```sh
reimu --file=student.s,runtime.s --memory=256M --stack=1M \
  --output=student.out
```

两个汇编输入都是文本 `.s` 文件；程序从标准输入读取并写入 `student.out`。REIMU 解析对其支持的 libc 函数（例如 `printf`、`scanf` 和 `malloc`）的调用。不会向 REIMU 传入目标文件或 ELF 二进制文件。自定义运行时应提供自己的汇编来代替 `runtime.s`。

源程序正常完成时返回状态 0，每个输出字节（包括空白字符）都是可观察的。

有效程序可能包含超出 12 位有符号立即数范围的整数常量和栈偏移，也可能包含超出单条条件分支指令跳转范围的控制流跳转。本规范不对源代码大小、编译时间、编译器内存或生成汇编的大小另设限制。

提交命令和评分政策属于课程管理内容，而非语言或执行语义。

<a id="optimization-and-execution-cost"></a>
## 优化与执行开销

不强制要求任何特定优化。只要能处理调用、有限数量的寄存器和所有有效控制流，基于栈槽的基线实现就是有效的。优化可以改变 IR、寄存器、存储和指令序列，但必须保留可观察的源程序行为和外部接口。

性能采用指定版本 REIMU 在默认权重下、未使用任何改变开销模型的选项时报告的 `Total cycles`。该计数包含生成代码、运行时指令和计费的 libc 操作。基线、汇总方式、分数和排名属于独立的考核政策。
