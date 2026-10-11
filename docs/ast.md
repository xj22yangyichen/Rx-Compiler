# AST 设计

节点定义在 [src/ast/ast.h](../src/ast/ast.h)，由 [src/frontend/ast-builder.cpp](../src/frontend/ast-builder.cpp) 从 CST 构建，[src/ast/ast-printer.cpp](../src/ast/ast-printer.cpp) 负责 JSON 输出，以便于调试和检验。

子节点用 `std::unique_ptr` 持有，同类节点的不同变体用 `std::variant` 区分。

## 位置信息

每个节点带一个 `Span`，`begin` / `end` 表示源码中的偏移，左闭右开；`line` / `column` 表示起始位置的行号和列号，从 1 开始。

## 节点设计

### 顶层

调用时指定的 `--entry` 决定根节点的类型：`crate` → `Crate`，`item` → `Item`，`expression` → `Expr`，`typeRef` → `Type`，`letStatement` → `Stmt`。

### Item

| 节点 | 字段 |
| --- | --- |
| `FunctionItem` | `name`、`receiver`（可选）、`parameters`、`return_type`（可为空）、`body` |
| `StructItem` | `name`、`derives`、`fields` |
| `ConstItem` | `name`、`type`、`value` |
| `ImplItem` | `type`、`items` |
| `UseItem` | 无（占位） |

辅助结构：`Receiver { is_reference, is_mutable }`、`Parameter { name, is_mutable, type }`、`StructField { name, type }`。

### Type

| 节点 | 字段 |
| --- | --- |
| `PathType` | `path` |
| `RefType` | `is_mutable`、`inner` |
| `ArrayType` | `element`、`length` |
| `UnitType` | 无 |

`Path` 是 `PathSegment` 的列表，每段含 `name` 和类型参数列表 `arguments`。

### Stmt

| 节点 | 字段 |
| --- | --- |
| `LetStmt` | `name`、`is_mutable`、`type`（可为空）、`init` |
| `ExprStmt` | `expression`、`has_semicolon` |
| `EmptyStmt` | 无 |

### Expr

| 节点 | 字段 |
| --- | --- |
| `IntegerExpr` | `spelling`（原文）、`digits`（去前缀和下划线）、`base`、`suffix` |
| `BoolExpr` | `is_true` |
| `UnitExpr` | 无 |
| `PathExpr` | `path` |
| `UnaryExpr` | `op`、`operand` |
| `BinaryExpr` | `op`、`left`、`right` |
| `AssignExpr` | `op`、`left`、`right` |
| `CastExpr` | `expression`、`type` |
| `CallExpr` | `callee`、`arguments` |
| `MethodCallExpr` | `receiver`、`method`（一个 `PathSegment`，可带类型参数）、`arguments` |
| `FieldExpr` | `object`、`name` |
| `IndexExpr` | `object`、`index` |
| `StructExpr` | `path`、`fields`（`FieldInit { name, value }` 列表） |
| `ArrayExpr` | `elements` |
| `RepeatExpr` | `element`、`length` |
| `BlockExpr` | `statements`、`tail`（可为空） |
| `IfExpr` | `condition`、`then_branch`、`else_branch`（可为空） |
| `LoopExpr` | `body` |
| `WhileExpr` | `condition`、`body` |
| `BreakExpr` | `value`（可为空） |
| `ContinueExpr` | 无 |
| `ReturnExpr` | `value`（可为空） |

运算符枚举 `Op` 覆盖一元（`-` `!` `*` `&` `&mut`）、二元（算术、位运算、移位、比较、逻辑）和赋值（`=` 及十种复合赋值）。

## 特殊说明

- g4 把表达式层级复制了四套（普通 / `condition` / `conditionBreak` / `statement`），AST 中合并为同一套节点。
- `&&x`、`&&T` 拆为两层引用；`>>`、`>=`、`>>=` 由 lexer 拆开的 token 片段重新拼接为运算符。
- 整数字面量解析出进制、去下划线的数字串和类型后缀，不做范围检查，留给 semantic 阶段。
- 块末尾不带分号的 `if` / `loop` / 块表达式在解析树中可能表现为语句，建树时统一识别为块的尾表达式。

## `builder` 设计

### 入口

**`build_root`**

根据 `--entry` 决定的起始规则，把解析树的根分派给对应的构建函数，返回 `Root`。

### 4 种类别节点的主构建函数

这四个函数对应 AST 的四种节点类别。

**`build_item`**

构建顶层声明。先把 `item` / `associatedItem` 去掉，再按实际类型处理：

- `use` 声明 → 只产生一个 `UseItem` 占位，内容整个丢弃
- 函数 → 名字、`self` 接收者（是否引用、是否 `mut`）、参数列表、返回类型、函数体（调 `build_block`）
- 结构体 → 名字、`#[derive(...)]` 列表、字段
- 常量 → 名字、类型、值
- `impl` 块 → 目标类型，然后对里面每个成员递归调 `build_item`

**`build_type`**

构建类型。处理的解析树节点有四种：

- `typeRef`：文法的类型总入口。如果是括号包裹的类型，去掉括号但保留外层 span；`()` 变成 `UnitType`；其余转发给路径 / 引用 / 数组分支
- `arrayType`：`[T; N]` → 元素类型 + 长度表达式
- `referenceType`：`&T` / `&mut T`。lexer 把 `&&` 切成一个 token，所以 `&&mut T` 要在这里手工拆成两层 `RefType`，`mut` 属于内层
- `closedCastType`：`as` 后面的类型

**`build_stmt`**

构建语句。`let` 语句提取名字、`mut`、可选类型标注和初始化表达式。普通 `statement` 规则有四种形态：包装的 `let`、带块的表达式（`if` / `loop` / 块，分号可有可无，所以要记录 `has_semicolon`）、普通表达式（必有分号）、单独一个分号。

**`build_expr`**

按解析树节点的规则编号把工作转给下面的函数。

### 表达式专项函数

`build_expr` 把工作分给这几个函数，每个对应运算符优先级的一层。

**`build_chain`**

处理二元运算和赋值。函数顺序扫描：遇到操作数就递归构建，遇到运算符就把它的文本累积到 `operator_text` 里，凑齐一对后用 `binaryOp` 查出 `Op` 枚举、合成一个新节点作为下一轮的左操作数。

为什么累积文本而不是直接取 token？因为 `>>`、`>=`、`>>=` 在 lexer 被拆成了 `>` + `>` + `=`。

赋值和二元运算共用这个函数，靠 `op >= Op::ASSIGN` 区分生成 `AssignExpr` 还是 `BinaryExpr`。

**`build_unary`**

处理一元运算：`-` `!` `*` `&` `&mut`。和上面一样，`&&x` / `&&mut x` 要拆成外层 `&` 加内层 `&` / `&mut`。

**`build_cast`**

处理 `as` 链：`x as u32 as i32`。

**`build_postfix`**

处理后缀链：从一个 primary 出发，后缀地套 `(args)` 调用、`[i]` 下标、`.field` 字段访问、`.method(args)` 方法调用。每套一层，当前结果就成为下一层的 `callee` / `object` / `receiver`。

**`build_primary`**

处理原子表达式。

### 其它工具

**`build_block`**

构建 `{ ... }`。逐条语句调 `build_stmt`，再处理尾表达式。注意到ANTLR会把 `{ if c { 1 } else { 2 } }` 匹配成一条 `statement` 而不是 `statementExpression`。所以如果文法没给出尾表达式，就检查最后一条语句——是表达式语句且无分号，就把它提升为 `tail`。见代码注释。

**`build_literal`**

`true` / `false` 直接生成 `BoolExpr`。整数字面量做纯文本拆解：先分离类型后缀（`i32` / `u32` / `isize` / `usize`），再分离进制前缀（`0x` / `0o` / `0b`），最后删掉下划线分隔符。保留字符串，数值转换和范围检查留给 semantic 阶段。

**`build_path`** / **`build_segment`**

`build_path` 把 `A::B::<T>::c` 这样的路径拆成段列表；`build_segment` 处理名字加上可选的类型参数列表 `<T, U>`。

**`build_arguments`**

把 `(a, b, c)` 里的每个表达式构建出来放进 vector。

### span

不属于 `class AstBuilder` 的成员。

**`tokenSpan`**

把一个 token 的位置转成 `Span`。`EOF` 是宽度为 0 需要特殊处理。

**`sourceSpan`**

把一个解析树节点的覆盖范围转成 `Span`：起点取首 token 的起点，终点取末 token 的终点。

## JSON 输出

`rxc --stage ast` 成功时把 AST 写到 stdout。文档根有三个键：`file`、`span_units`（固定为 `unicode-codepoints`）和 `root`。每个节点带 `kind` 和 `span`，其余键与上表的字段名一致，例外是 `BoolExpr` 的 `is_true` 输出为 `value`，`Op` 输出为运算符原文。空指针输出为 `null`。

对表达式 `a + 1`（`--entry expression`）的完整输出：

```json
{
  "file": "e.rx",
  "span_units": "unicode-codepoints",
  "root": {
    "kind": "BinaryExpr",
    "span": {
      "begin": 0,
      "end": 5,
      "line": 1,
      "column": 1
    },
    "op": "+",
    "left": {
      "kind": "PathExpr",
      "span": {
        "begin": 0,
        "end": 1,
        "line": 1,
        "column": 1
      },
      "path": [{
        "kind": "PathSegment",
        "span": {
          "begin": 0,
          "end": 1,
          "line": 1,
          "column": 1
        },
        "name": "a",
        "arguments": []
      }]
    },
    "right": {
      "kind": "IntegerExpr",
      "span": {
        "begin": 4,
        "end": 5,
        "line": 1,
        "column": 5
      },
      "spelling": "1",
      "digits": "1",
      "base": 10,
      "suffix": ""
    }
  }
}
```
