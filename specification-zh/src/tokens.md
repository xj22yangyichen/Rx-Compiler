r[lex.token]
# 词法单元

r[lex.token.syntax]
```grammar,lexer
@root Token ->
      IDENTIFIER_OR_KEYWORD
    | INTEGER_LITERAL
    | LIFETIME_TOKEN
    | PUNCTUATION
```

词法单元包括标识符或关键字、整数字面量、生命周期词法单元和标点符号。关键字（包括布尔字面量 `true` 和 `false`）与标识符共享词法字符模式，但它们的语法角色由关键字规则和 [LiteralExpression] 产生式决定。

词法分析会在满足下述数字和生命周期边界的前提下选择最长词法单元。解析器可以根据[上下文标点](grammar.md#contextual-punctuation)读取组合标点的前缀。

## 整数字面量

r[lex.token.literal.int.syntax]
```grammar,lexer
INTEGER_LITERAL ->
    ( DEC_LITERAL | BIN_LITERAL | OCT_LITERAL | HEX_LITERAL ) INTEGER_SUFFIX?

DEC_LITERAL -> DEC_DIGIT (DEC_DIGIT|`_`)*

BIN_LITERAL -> `0b` (BIN_DIGIT|`_`)* BIN_DIGIT (BIN_DIGIT|`_`)*

OCT_LITERAL -> `0o` (OCT_DIGIT|`_`)* OCT_DIGIT (OCT_DIGIT|`_`)*

HEX_LITERAL -> `0x` (HEX_DIGIT|`_`)* HEX_DIGIT (HEX_DIGIT|`_`)*

BIN_DIGIT -> [`0`-`1`]

OCT_DIGIT -> [`0`-`7`]

DEC_DIGIT -> [`0`-`9`]

HEX_DIGIT -> [`0`-`9` `a`-`f` `A`-`F`]
```

```grammar,lexer
INTEGER_SUFFIX -> `i32` | `u32` | `isize` | `usize`
```

整数字面量使用十进制数字，或带二进制、八进制、十六进制前缀的数字，并可以使用下划线分隔符。非十进制字面量的前缀后至少必须有一个数字。四种整数后缀分别指定对应的整数类型。

| Form | Examples |
| --- | --- |
| Decimal | `123`, `1_234`, `123_i32` |
| Binary | `0b1010`, `0b____1`, `0b10_u32` |
| Octal | `0o77`, `0o7_usize` |
| Hexadecimal | `0xff`, `0xAB_CD`, `0xff_isize` |

词法分析器使用最长匹配读取完整的数字词法单元。对于无效后缀或无效进制数字，词法分析器不得将输入拆成有效整数字面量和后续词法单元来恢复；`123i32foo`、`123bad`、`0b102` 和 `0x` 等写法必须作为无效词法单元拒绝。提供的 Rust 词法分析器可以保留通用后缀，并在语言子集边界拒绝不受支持的后缀。

词法单元长度和数值大小均没有限制。编译器必须保留原始数字和后缀，不得要求字面量的数值能装入宿主平台的整数类型。类型确定和范围有效性遵循[字面量表达式](expressions/literal-expr.md#integer-typing-and-range)。

超出其确定类型范围的字面量属于未定义行为；见[整数字面量范围保证](undefined-behavior.md#integer-literal-range)。

前导减号（`-`）作为独立运算符进行词法分析，而不是整数字面量的一部分。由于 `f`、`3` 和 `2` 都是有效的十六进制数字，`0x01_f32` 会被解析为单个十六进制整数字面量，其值为 `0x01f32` 且没有后缀，而不是浮点字面量。

## 生命周期

r[lex.token.life.syntax]
```grammar,lexer
LIFETIME_TOKEN -> `'` IDENTIFIER_OR_KEYWORD

LIFETIME_OR_LABEL -> `'` NON_KEYWORD_IDENTIFIER
```

生命周期词法单元由紧跟名称的撇号组成，中间不能有空白或注释。词法分析器读取完整名称。示例包括 `'a`、`'data`、`'static` 和 `'_'`。生命周期词法单元后不能再出现结束撇号；`'a'` 这样的写法表示字符字面量，而 Rx 不支持字符字面量。

[LIFETIME_OR_LABEL] 向 [Lifetime] 提供普通的具名生命周期。特殊写法 `'static` 和 `'_'` 在那里有各自的可选形式。生命周期词法单元可以出现在引用类型、参数声明、路径实参和约束中。尽管词法产生式 `LIFETIME_OR_LABEL` 能匹配循环标签词法单元，Rx 控制流产生式（[Loops](expressions/loop-expr.md)）不支持带标签的循环或跳转。

## 标点符号

r[lex.token.punct.syntax]
```grammar,lexer
PUNCTUATION ->
      `=` | `<` | `<=` | `==` | `!=` | `>=` | `>`
    | `&&` | `||` | `!`
    | `+` | `-` | `*` | `/` | `%` | `^` | `&` | `|` | `<<` | `>>`
    | `+=` | `-=` | `*=` | `/=` | `%=` | `^=` | `&=` | `|=` | `<<=` | `>>=`
    | `.` | `,` | `;` | `:` | `::` | `->` | `#` | `_`
    | `{` | `}` | `[` | `]` | `(` | `)`
```

注释优先于 `/` 标点。组合运算符遵循最长词法单元规则；它们在上下文中的拆分方式见[解析器约定](grammar.md#contextual-punctuation)。

Rx 不支持浮点数；包含小数或指数部分的数字词法单元（例如 `1.0` 或 `1e5`）必须作为词法错误拒绝，而不能拆成整数和标点词法单元。分隔符（括号、方括号和花括号）必须始终成对出现。`#` 字符专门用于引入外部 derive 属性。
