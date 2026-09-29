# Rx 编译器规范

欢迎来到 Rx 编译器课程！在本课程中，你需要为 [Rust](https://www.rust-lang.org/) 的一个子集编写编译器，我们将这个子集称为 *Rx*。编译器的目标是 [RV32IM](https://en.wikipedia.org/wiki/RISC-V) 指令集架构。为简化前端开发，我们提供了 Rx 的 [ANTLR v4](https://www.antlr.org/) 语法，这会自动完成解析，使你可以直接从抽象语法树（AST）开始工作。

编译器必须使用 LLVM IR 作为中间表示，并支持将其输出为文本。这使你可以在实现自己的 RISC-V 后端之前，使用 Clang 验证前端，具体方法见[后端](backend.md#llvm-ir-and-code-generation)。

为兼容 Rust，解析器还接受顶层 `use` 声明和生命周期语法。你可以[在解析后丢弃这两者](grammar.md#syntax-that-may-be-discarded-after-parsing)；导入解析和生命周期检查不属于本课程作业。

本书严格定义 Rx 的语法和语义，并与官方 Rust 编译器（`rustc`）一起作为本课程的依据。如果你发现两者之间存在歧义或不一致，请及时向助教寻求澄清。

## 如何阅读规范

请先阅读[语言范围与测试保证](undefined-behavior.md)，其中区分了有效程序、编译错误和排除的情况（即*未定义行为*），帮助你了解哪些功能是必需的，哪些超出范围。

接下来阅读[词法结构](lexical-structure.md)，然后依次阅读[条目](items.md)、[类型](types.md)和[表达式](expressions.md)。这些章节定义 Rx 语言的完整语法。[语法汇总](grammar-summary.md)一节提供了便于快速查阅的表格。

阅读时，我们建议使用 Rust 编译器（`rustc`）进行实验。你也可以使用 [Rust Playground](https://play.rust-lang.org/?version=stable&mode=debug&edition=2024) 或 [Compiler Explorer](https://godbolt.org/) 等在线环境，观察哪些代码可以编译、哪些不可以。请注意，我们**不将编译器警告视为编译错误**。

## 如何开始

开始之前，建议先复习 Rust 入门教程。由于 Rx 只实现 Rust 的一个子集，你不需要全面掌握 Rust，但熟悉基本 Rust 语法是必需的。

网上有许多优秀资料介绍编译器构造的不同阶段。以下是一些由你的助教（或他们的助教）撰写的博客：

- [Compiler for Mx* 编译器随笔](https://darksharpness.top/CompilerMx/)：传奇助教 [@darksharpness](https://darksharpness.top/) 的博客文章，重点介绍 Mx（Rx 之前使用的课程语言）中的各种优化技术。
- [Writing a Compiler for Rust](https://norb.space/blog/writing-a-compiler-for-rust)：由本规范共同设计者 RayZh 撰写，主要介绍端到端的编译器流水线。请注意，由于今年规范有所更新，其中讨论的一些技术已不再需要或不再适用。

更多资料：

- [Compiler 101](https://github.com/ACMClassCourses/Compiler-101)：ACM Class 学生多年来积累的项目和笔记集合。请注意，所述技术中有许多针对 Mx，可能不适用于 Rx。
- [The Rust Reference](https://doc.rust-lang.org/reference/)：Rust 语言的官方完整参考。尽管本 Rx 规范力求自洽，《Rust 参考手册》仍是标准 Rust 语义的权威补充指南。
- [Rust Compiler Development Guide](https://rustc-dev-guide.rust-lang.org/)：深入介绍 `rustc` 内部实现和架构的高级指南，适合作为编译器工程模式的背景参考。
