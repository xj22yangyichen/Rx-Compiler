<a id="use-declarations"></a>
# `use` 声明

```grammar,items
UseDeclaration -> `use` UseTree `;`

UseTree ->
      (UsePath? `::`)? (`*` | `{` (UseTree (`,` UseTree)* `,`?)? `}`)
    | UsePath (`as` (IDENTIFIER | `_`))?

UsePath -> `::`? UsePathSegment (`::` UsePathSegment)*

UsePathSegment -> IDENTIFIER | `self` | `super` | `crate`
```

接受 use 声明作为顶层条目，以便同一份测试源代码也能使用 Rust 和课程支持 crate 编译。例如：

```rust,ignore
use rx::core::*;

fn main() {
    println_i32(42);
}
```

该语法接受单独的路径、`*` 通配导入、允许可选尾随逗号的嵌套花括号组，以及 `as` 别名。例如，也接受 `use rx::core::println_i32;` 和 `use rx::core::{get_i32, println_i32};`。Use 声明不能出现在表达式块或 `impl` 块内，也不接受可见性修饰符或属性。

解析完成后，编译器可以丢弃整个声明。编译器不会解析导入路径、加载 crate 或模块、向任何作用域引入名称或别名、检查导入冲突，也不会为该声明生成 LLVM IR。无论是否存在 `use` 声明，`println_i32` 等内建条目在 Rx 中都已经可用。

[use 兼容性保证](../undefined-behavior.md#use-compatibility)确保测试不依赖额外的导入语义。use 声明接受的扩展路径语法（包括多段路径、前导 `::`、`crate` 和 `super`）不适用于[类型和表达式路径](../paths.md)。另见[解析后可丢弃的语法](../grammar.md#syntax-that-may-be-discarded-after-parsing)。
