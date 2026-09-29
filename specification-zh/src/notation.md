# 记法

## 语法

以下记法用于 *Lexer* 和 *Syntax* 语法片段：

| Notation          | Examples                      | Meaning                                   |
|-------------------|-------------------------------|-------------------------------------------|
| CAPITAL           | IDENTIFIER, INTEGER_LITERAL        | 词法分析器产生的词法单元             |
| _ItalicCamelCase_ | _LetStatement_, _Item_        | 语法产生式                  |
| `string`          | `x`、`while`、`*`             | 确切的字符                    |
| x<sup>?</sup>     | `mut`<sup>?</sup>             | 可选项                          |
| x<sup>*</sup>    | _OuterAttribute_<sup>*</sup> | 0 个或多个 x                            |
| x<sup>+</sup>     |  _Statement_<sup>+</sup>     | 1 个或多个 x                            |
| x<sup>a..b</sup>  | HEX_DIGIT<sup>1..6</sup>      | x 重复 a 到 b 次                   |
| Rule1 Rule2       | `fn` _Name_ _Parameters_      | 按顺序排列的规则序列                |
| \|                | `i32` \| `u32`、Block \| Item  | 选择（匹配其中一个可选项） |
| `[ ]`              | `[b B]`                       | 列出的任意字符              |
| `[ - ]`            | `[a-z]`                       | 范围内的任意字符        |
| `~[ ]`             | `~[b B]`                      | 除列出字符之外的任意字符       |
| ~`string`         | ~`\n`、~`*/`                  | 除该序列之外的任意字符      |
| ( )               | (`,` _Parameter_)<sup>?</sup> | 将项目分组                              |
| U+xxxx            | U+0060                        | 单个 Unicode 字符                |
| \<text\>          | \<any ASCII char except CR\>  | 对应匹配内容的英文描述 |
| Rule <sub>suffix</sub> | IDENTIFIER_OR_KEYWORD <sub>_except `crate`_</sub> | 对上一规则的修改 |

顺序组合的优先级高于 `|` 选择。

<details>
<summary>字符串表产生式</summary>

<a id="string-table-productions"></a>
## 字符串表产生式

某些规则，包括[一元运算符]、[二元运算符]和[关键字]，以字面字符串表的形式定义。在语法产生式中，对字面字符串的引用会隐式匹配词法分析产生的对应词法单元。参见[词法单元][词法单元]。

</details>

## 语法可视化

语法产生式出现在定义它们的章节中。词法规则使用全大写名称；语法规则使用大小写混合的名称。每个语法块下方的按钮会显示其[语法图]，其中矩形表示非终结符，圆角矩形表示终结符。

[语法图]: https://en.wikipedia.org/wiki/Syntax_diagram

## 通用产生式

以下是语法中使用的通用定义。

r[input.syntax]
```grammar,lexer
CHAR -> <a 7-bit ASCII source character>

SPACE -> U+0020

TAB -> U+0009

LF -> U+000A

CR -> U+000D
```

[二元运算符]: expressions/operator-expr.md#arithmetic-and-bits
[关键字]: keywords.md
[词法单元]: tokens.md
[一元运算符]: expressions/operator-expr.md#borrow-dereference-and-assignment
