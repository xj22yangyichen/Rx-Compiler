# Rx Compiler

Rx 语言编译器，ACM 班编译器课程项目。Rx 是 Rust 2021 的一个子集，编译目标为 RISC-V RV32IM 汇编，在 [REIMU](https://github.com/wanoful/REIMU) 模拟器上运行。本项目使用 C++17 实现。

语言规范见 [specification/](specification/)，中文翻译见 [specification-zh/](specification-zh/)。官方测试用例见 [tests/official/](tests/official/)。

中文版规范预览：先运行命令

```sh
make -C specification-zh serve
```

然后访问 http://127.0.0.1:3000/。

英文版规范在线预览：[Rx Compiler Specification](https://acmclasscourse-2025.github.io/rx-compiler-specification/)。

## 项目进度

| 阶段 | 内容 | 状态 |
| --- | --- | --- |
| 前端 | 词法 / 语法分析，错误检测，AST 构建 | 完成 |
| 语义分析 | 名字解析、类型检查、可变性检查、常量求值 | 完成 |
| IR | 生成 LLVM IR 文本 | 未开始 |
| 代码生成 | 自实现后端，输出 RV32IM 汇编 | 未开始 |
| 优化 | 寄存器分配、内联、死代码消除、常量传播、尾递归、除法取模优化 | 未开始 |

课程验收节点：第 4 周 AST，第 8 周 IR，第 12 周代码生成，第 16 周优化。

## 目录结构

```text
src/
  main.cpp                 命令行入口
  generated/               预生成的 ANTLR C++ 解析器源码
  frontend/
    frontend.h / .cpp      词法 → 语法 → AST 的流水线入口
    error-listener.h / .cpp ANTLR 错误监听器
    diagnostic.h           诊断信息结构
    ast-builder.h / .cpp   ANTLR 解析树 → AST
  ast/
    ast.h / .cpp           AST 节点定义
    ast-printer.h / .cpp   AST 的 JSON 输出（调试用）
  semantic/
    analyzer.h / .cpp      语义分析入口、声明收集、类型解析与常量求值
    type.h / .cpp          语义类型表示与类型相等判定
    check-stmt.cpp         函数体、块、语句与局部绑定检查
    check-expr.cpp         表达式类型、可变性、调用与类型转换检查
docs/
  ast.md                   AST 节点设计与 JSON 输出格式
  semantic.md              语义分析的职责、设计与工作流程
grammar/                   官方提供的 ANTLR 文法（Lexer.g4, Parser.g4），不做修改
tests/
  official/                官方测试用例（testcases/ 的副本）
  custom/frontend/         自定义的词法 / 语法诊断用例
  custom/ast/              自定义的 AST 结构用例
scripts/
  test-frontend.py         前端测试脚本
  test-ast.py              AST 测试脚本
  test.py                  官方测试运行器（由 Makefile 调用）
CMakeLists.txt             C++ 构建配置
Makefile / config.mk       官方测试框架入口
vendor/REIMU               REIMU 模拟器（后续阶段使用）
crates/rx                  官方提供的 rustc 参考实现辅助代码
```

## 环境依赖

| 工具 | 用途 | 备注 |
| --- | --- | --- |
| g++-14 | 编译本项目 | 需要 C++17 |
| CMake ≥ 3.20 | 构建 | |
| ANTLR C++ runtime | 编译和运行词法 / 语法分析器 | CI 使用 4.13.2 |
| Java + ANTLR 4.13.2 jar | 重新生成词法 / 语法分析器 | 仅更新文法时需要 |
| Python 3 | 测试脚本 | |
| xmake + REIMU | 运行生成的汇编 | 后续阶段 |
| Clang 22 | IR 阶段测试 | 后续阶段 |

普通构建直接使用 `src/generated/` 中随仓库提供的解析器源码，不需要 Java、ANTLR jar 或联网下载。CMake 会查找系统安装的 ANTLR C++ runtime，也支持 `~/.local/include/antlr4-runtime` 和 `~/.local/lib`；可用 `ANTLR_INCLUDE_DIR`、`ANTLR_LIBRARY` 指定其他位置。

## 构建

```sh
cmake -S . -B target/frontend -DCMAKE_CXX_COMPILER=g++-14
cmake --build target/frontend -j2
```

生成的可执行文件为 `target/frontend/rxc`。

### 更新解析器

文法更新后，在有 Java 的开发环境中重新生成解析器：

```sh
cmake -S . -B target/parser-generator -DCMAKE_CXX_COMPILER=g++-14 \
  -DRX_REGENERATE_PARSER=ON \
  -DANTLR_JAR="$HOME/.local/lib/antlr-4.13.2-complete.jar"
cmake --build target/parser-generator --target rx_parser -j2
cp target/parser-generator/generated/RxLexer.cpp src/generated/rx-lexer.cpp
cp target/parser-generator/generated/RxLexer.h src/generated/rx-lexer.h
cp target/parser-generator/generated/RxParser.cpp src/generated/rx-parser.cpp
cp target/parser-generator/generated/RxParser.h src/generated/rx-parser.h
```

生成时将文法副本改名为 `RxLexer` / `RxParser`，避免与 ANTLR 运行时类名冲突，原始文法不变。预生成文件保留生成器输出的内容；普通构建会将它们复制为构建目录中的 `RxLexer.*`、`RxParser.*`，供现有代码引用。更新后应将四个生成文件与文法改动一同提交。

OJ 的 C++ 环境没有 Java 时使用默认构建即可，提交时必须包含 `src/generated/` 中的四个文件，并确保评测环境提供兼容的 ANTLR C++ runtime。

## 使用

```sh
rxc [--stage lex|parse|ast|semantic] [--entry crate|expression|typeRef|item|letStatement] <source.rx>
```

| 选项 | 说明 | 默认值 |
| --- | --- | --- |
| `--stage` | `lex` 做词法检查；`parse` 增加语法检查；`ast` 构建并输出 AST；`semantic` 在 AST 上进行语义检查 | `parse` |
| `--entry` | 语法分析的起始规则。`crate` 为完整程序，其余为语法片段；`semantic` 阶段仅支持 `crate` | `crate` |

退出码：**0** 表示接受，**1** 表示源码在所选阶段的检查中存在错误。诊断信息输出到 stderr，格式为：

```text
bad.rx:3:1: parse error: missing ';' at '}'
```

词法、语法、语义错误分别标记为 `lex error` / `parse error` / `semantic error`。词法阶段出错时不再进入语法分析，词法和语法检查均通过后才构建 AST 并进入语义分析。即便 ANTLR 的错误恢复机制返回了一棵解析树，只要产生过诊断就判定为拒绝。

`--stage ast` 成功时把 AST 以 JSON 形式写到 stdout，节点结构和输出格式见 [docs/ast.md](docs/ast.md)。

语义检查完整程序：

```sh
./target/frontend/rxc --stage semantic program.rx
```

当前 `semantic` 阶段只要构建出 AST，也会向 stdout 输出 AST JSON，包括随后发现语义错误的情况；程序是否被接受应以退出码为准。语义分析的设计与流程见 [docs/semantic.md](docs/semantic.md)。

命令行参数不做校验，只支持上述合法形式。注意源文件不存在时不会报错，而是按空输入处理。

## 测试

### 前端测试

```sh
python3 scripts/test-frontend.py --parse-regressions
```

| 分组 | 内容 | 用例数 |
| --- | --- | --- |
| `official:lex` | 官方 lexer 测试，按 `compilation_success` 检查退出码 | 53 |
| `official:parse` | 官方 parser 测试，按清单中的 `entry` 选择起始规则 | 442 |
| `custom:diagnostics` | 自定义用例，检查诊断的位置、内容和条数 | 26 |
| `parse-only:*` | 对 semantic / codegen / optimization 的全部源文件做纯语法检查；负例允许在语法阶段被拒绝 | 312 |

不加 `--parse-regressions` 时跳过最后一组。

### AST 测试

```sh
python3 scripts/test-ast.py
```

| 分组 | 内容 | 用例数 |
| --- | --- | --- |
| `official:*` | 按用例预期检查 AST 输出；构建成功时检查 JSON、span 和行列号；运行阶段负例允许在语法阶段被拒绝 | 754 |
| `official:lex-errors` | 词法错误的源文件不得输出 AST | 23 |
| `custom:structure` | 结构断言：优先级、结合性、块尾表达式、双重引用、类型参数等 | 27 |
| `custom:operators` | 全部一元、二元、赋值运算符 | 34 |
| `custom:expression-families` | 同一表达式在普通 / 条件 / 语句等不同文法位置下生成相同的 AST | 48 |
| `custom:error-gating` | 各入口下有语法错误时不输出 AST | 6 |
| `custom:json-escaping` | 文件名含特殊字符时 JSON 仍合法 | 1 |

自定义用例的数据文件在 `tests/custom/frontend/cases.json` 和 `tests/custom/ast/cases.json`。

两个脚本都支持 `--compiler <path>` 指定可执行文件、`--timeout <秒>` 调整单例超时。

### 官方测试框架

运行当前已实现的语义分析测试：

```sh
make test STAGE=semantic
```

当前官方 semantic 测试共 239 个用例，全部通过。框架通过退出码检查接受或拒绝结果。

`make test` 调用 [config.mk](config.mk) 中配置的命令运行 semantic / ir / codegen / optimization 测试。`BUILD` 和 `SEMANTIC` 已接入本项目的 C++ 编译器 `rxc`；`IR` 和 `CODEGEN` 仍使用 rustc 参考配置，`RUN` 使用 REIMU，这些配置不代表自实现的后续阶段已经完成。框架用法详见 [README-ZH.md](README-ZH.md)。

官方框架会跳过 lex 和 parse 类型的测试，这两类由上面的前端测试脚本覆盖。
