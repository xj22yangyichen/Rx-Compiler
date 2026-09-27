# Rx Compiler Specification

Welcome to the Rx compiler project! In this course, you will write a compiler for a subset of Rust. This repository provides the specification for the Rx language you will implement, covering both its grammar and runtime semantics.

## Online preview

Read the specification online at [Rx Compiler Specification](https://acmclasscourse-2025.github.io/rx-compiler-specification/).

## Build and preview

For local use, clone this repository and build the specification using `mdbook`.

The book uses mdBook 0.4.x (CI pins 0.4.48) and the bundled `mdbook-spec` preprocessor.

```sh
cargo install --locked mdbook --version 0.4.48
SPEC_RELATIVE=0 SPEC_DENY_WARNINGS=1 mdbook build
```

The generated site is written to `book/`. For a local preview:

```sh
SPEC_RELATIVE=0 mdbook serve --open
```

## Issues and contributing

If you identify any issues with the specification, or any discrepancies between this specification and the [Rust Reference](https://doc.rust-lang.org/stable/reference/) that lead to incorrect behavior for defined (non-UB) programs, please report them in the [issues section](https://github.com/acmclasscourse-2025/rx-compiler-specification/issues).

## License

This project is dual-licensed under the MIT and Apache 2.0 licenses. See [LICENSE-MIT](LICENSE-MIT) and [LICENSE-APACHE](LICENSE-APACHE) for details.
