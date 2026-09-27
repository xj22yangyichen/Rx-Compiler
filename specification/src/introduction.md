# Rx Compiler Specification

Welcome to the Rx Compiler course! In this course, you are required to write a compiler for a subset of [Rust](https://www.rust-lang.org/) we call *Rx*. The compiler will target the [RV32IM](https://en.wikipedia.org/wiki/RISC-V) instruction set architecture. To simplify frontend development, we provide an [ANTLR v4](https://www.antlr.org/) grammar for Rx that automates parsing, allowing you to begin directly with an Abstract Syntax Tree (AST).

The compiler must use LLVM IR as its intermediate representation and support emitting it as text. This lets you validate the frontend using Clang before implementing your own RISC-V backend, as described in [Backend](backend.md#llvm-ir-and-code-generation).

For Rust compatibility, the parser also accepts top-level `use` declarations and lifetime syntax. You may [discard both after parsing](grammar.md#syntax-that-may-be-discarded-after-parsing); import resolution and lifetime checking are not part of the assignment.

This book rigorously defines the syntax and semantics of Rx and, alongside the official Rust compiler (`rustc`), serves as the source of truth for this course. If you find any ambiguities or disagreements between the two, do not hesitate to reach out to your TA for clarification.

## How to read the specification

Start by reading [Language scope and test guarantees](undefined-behavior.md), which distinguishes valid programs, compile errors, and excluded cases (i.e., *undefined behavior*) to understand which features are required and which are out of scope.

Next, proceed to [lexical structure](lexical-structure.md), followed by [items](items.md), [types](types.md), and [expressions](expressions.md). These sections define the complete grammar of the Rx language. There is a quick lookup table in the [grammar summary](grammar-summary.md) section.

While reading, we recommend experimenting with the Rust compiler (`rustc`). You can also use online environments such as the [Rust Playground](https://play.rust-lang.org/?version=stable&mode=debug&edition=2024) or [Compiler Explorer](https://godbolt.org/) to see what compiles and what does not. Note that we **do not treat compiler warnings as compile errors**.

## How to get started

Before beginning, we recommend reviewing an introductory Rust tutorial. You do not need an exhaustive understanding because Rx implements only a subset, but familiarity with basic Rust syntax is essential.

Many excellent online resources explain the various phases of compiler construction. Some blogs from your TAs (or their TAs) include:

- [Compiler for Mx* 编译器随笔](https://darksharpness.top/CompilerMx/) by legendary TA [@darksharpness](https://darksharpness.top/): A classic blog post that focuses heavily on various optimization techniques in Mx (the course language used prior to Rx).
- [Writing a Compiler for Rust](https://norb.space/blog/writing-a-compiler-for-rust) by RayZh (co-designer of this specification): Focuses primarily on the end-to-end compiler pipeline. Note that certain techniques discussed are no longer needed or applicable due to updates in this year's specification.

Additional information:

- [Compiler 101](https://github.com/ACMClassCourses/Compiler-101): A collection of projects and notes by ACM Class students over the years. Note that many of the techniques described are specific to Mx and may not apply to Rx.
- [The Rust Reference](https://doc.rust-lang.org/reference/): The official, comprehensive reference for the Rust language. While this Rx specification is intended to be self-contained, the Rust Reference is an authoritative supplementary guide to standard Rust semantics.
- [Rust Compiler Development Guide](https://rustc-dev-guide.rust-lang.org/): An advanced, in-depth guide to `rustc` internals and architecture, recommended as a background reference for compiler engineering patterns.
