# Crate 与源文件

```grammar,items
@root Crate -> Item*
```

一个 Rx 程序由单个源代码编译单元组成，其中包含一系列顶层条目。它的函数、结构体、常量和固有 impl 共享[名称](names.md)中描述的命名空间。

顶层 [Item] 的可选形式包括 `use` 声明、函数、具名字段结构体、常量和固有 `impl` 块。[use 声明](items/use-declarations.md)仅为兼容 Rust 而存在，可以在解析后丢弃；它们不会向 Rx 命名空间引入任何名称。不能在表达式块中声明条目。程序入口遵循[main 契约](undefined-behavior/builtin.md#program-entry)。
