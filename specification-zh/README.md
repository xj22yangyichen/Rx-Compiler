<a id="rx-compiler-specification"></a>

# Rx 编译器规范

欢迎来到 Rx 编译器项目！在本课程中，你将为 Rust 的一个子集编写编译器。本目录提供你将要实现的 Rx 语言规范，涵盖其语法和运行时语义。

<a id="online-preview"></a>

## 在线预览

可在 [Rx 编译器规范](https://acmclasscourse-2025.github.io/rx-compiler-specification/) 在线阅读原版规范。

<a id="build-and-preview"></a>

## 构建与预览

如需在本地使用，请克隆本仓库，并使用 `mdbook` 构建规范。运行以下命令前，请先进入本目录（`specification-zh/`）。

本书使用 mdBook 0.4.x（CI 固定为 0.4.48）以及随附的 `mdbook-spec` 预处理器。

```sh
cargo install --locked mdbook --version 0.4.48
SPEC_RELATIVE=0 SPEC_DENY_WARNINGS=1 mdbook build
```

生成的网站将写入 `book/`。如需本地预览：

```sh
make serve
```

默认地址为 http://127.0.0.1:3000/。也可以通过 `HOST` 和 `PORT` 覆盖监听地址和端口，例如 `make serve HOST=0.0.0.0 PORT=4000`。

<a id="issues-and-contributing"></a>

## 问题反馈与贡献

如果你发现规范中存在任何问题，或者本规范与 [Rust 参考手册](https://doc.rust-lang.org/stable/reference/) 之间存在差异，导致行为有定义的程序（即不含未定义行为的程序）出现错误行为，请在 [Issues 页面](https://github.com/acmclasscourse-2025/rx-compiler-specification/issues) 报告。

<a id="license"></a>

## 许可证

本项目采用 MIT 和 Apache 2.0 双重许可证。详情请参阅 [LICENSE-MIT](LICENSE-MIT) 和 [LICENSE-APACHE](LICENSE-APACHE)。许可证保留原文。

## 中文版说明

本目录是 `specification/` 的简体中文译本，保留原有目录结构。规范正文、目录及辅助文档已翻译；程序代码、技术标识符、构建工具、主题资源以及许可证原文保留，以免改变程序行为或法律文本含义。阅读入口为 [目录](src/SUMMARY.md)。
