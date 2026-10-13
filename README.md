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
| 语义分析 | 名字解析、类型检查、可变性检查 | 未开始 |
| IR | 生成 LLVM IR 文本 | 未开始 |
| 代码生成 | 自实现后端，输出 RV32IM 汇编 | 未开始 |
| 优化 | 寄存器分配、内联、死代码消除、常量传播、尾递归、除法取模优化 | 未开始 |

课程验收节点：第 4 周 AST，第 8 周 IR，第 12 周代码生成，第 16 周优化。

## 目录结构

```text
src/
  main.cpp                 命令行入口
  frontend/
    frontend.h / .cpp      词法 → 语法 → AST 的流水线入口
    error-listener.h / .cpp ANTLR 错误监听器
    diagnostic.h           诊断信息结构
    ast-builder.h / .cpp   ANTLR 解析树 → AST
  ast/
    ast.h / .cpp           AST 节点定义
    ast-printer.h / .cpp   AST 的 JSON 输出（调试用）
docs/
  ast.md                   AST 节点设计与 JSON 输出格式
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
| Java | 运行 ANTLR 生成工具 | 构建时自动调用 |
| ANTLR 4.13.2 | 生成词法 / 语法分析器 | jar 包与 C++ 运行时库 |
| Python 3 | 测试脚本 | |
| xmake + REIMU | 运行生成的汇编 | 后续阶段 |
| Clang 22 | IR 阶段测试 | 后续阶段 |

ANTLR 的安装位置：jar 包默认在 `~/.local/lib/antlr-4.13.2-complete.jar`，可通过 CMake 变量 `ANTLR_JAR` 覆盖；C++ 运行时的头文件在 `~/.local/include/antlr4-runtime`，库文件在 `~/.local/lib`。

构建时 CMake 会把 `grammar/` 下的文法复制一份并改名为 `RxLexer` / `RxParser`（避免与 ANTLR 运行时自身的 `antlr4::Lexer` / `antlr4::Parser` 类名冲突），再调用 ANTLR 生成 C++ 代码到构建目录。原始文法文件不会被修改。

## 构建

```sh
cmake -S . -B target/frontend -DCMAKE_CXX_COMPILER=g++-14
cmake --build target/frontend -j2
```

生成的可执行文件为 `target/frontend/rxc`。

## 使用

```sh
rxc [--stage lex|parse|ast] [--entry crate|expression|typeRef|item|letStatement] <source.rx>
```

| 选项 | 说明 | 默认值 |
| --- | --- | --- |
| `--stage` | `lex` 只做词法检查；`parse` 做词法 + 语法检查；`ast` 在此基础上构建 AST 并输出 JSON | `parse` |
| `--entry` | 语法分析的起始规则。`crate` 为完整程序，其余为语法片段，用于官方 parser 测试 | `crate` |

退出码：**0** 表示接受，**1** 表示源码存在词法或语法错误。诊断信息输出到 stderr，格式为：

```text
bad.rx:3:1: parse error: missing ';' at '}'
```

词法错误和语法错误会分别标记为 `lex error` / `parse error`。词法阶段出错时不再进入语法分析。即便 ANTLR 的错误恢复机制返回了一棵解析树，只要产生过诊断就判定为拒绝。

`--stage ast` 成功时把 AST 以 JSON 形式写到 stdout，节点结构和输出格式见 [docs/ast.md](docs/ast.md)。

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
| `parse-only:*` | 对 semantic / codegen / optimization 的全部源文件做纯语法检查 | 309 |

不加 `--parse-regressions` 时跳过最后一组。

### AST 测试

```sh
python3 scripts/test-ast.py
```

| 分组 | 内容 | 用例数 |
| --- | --- | --- |
| `official:*` | 对所有官方源文件构建 AST，检查 JSON 合法、span 嵌套正确、行列号与源码一致 | 751 |
| `official:lex-errors` | 词法错误的源文件不得输出 AST | 23 |
| `custom:structure` | 结构断言：优先级、结合性、块尾表达式、双重引用、类型参数等 | 27 |
| `custom:operators` | 全部一元、二元、赋值运算符 | 34 |
| `custom:expression-families` | 同一表达式在普通 / 条件 / 语句等不同文法位置下生成相同的 AST | 48 |
| `custom:error-gating` | 各入口下有语法错误时不输出 AST | 6 |
| `custom:json-escaping` | 文件名含特殊字符时 JSON 仍合法 | 1 |

自定义用例的数据文件在 `tests/custom/frontend/cases.json` 和 `tests/custom/ast/cases.json`。

两个脚本都支持 `--compiler <path>` 指定可执行文件、`--timeout <秒>` 调整单例超时。

### 官方测试框架

`make test` 调用 [config.mk](config.mk) 中配置的命令运行 semantic / ir / codegen / optimization 测试。目前 `config.mk` 仍是官方模板给出的 rustc 参考配置，待语义分析阶段完成后再接入 `rxc`。用法详见 [README-ZH.md](README-ZH.md)。

官方框架会跳过 lex 和 parse 类型的测试，这两类由上面的前端测试脚本覆盖。
