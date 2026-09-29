<a id="grammar"></a>

# 语法

参考语法以 Markdown 代码块编写，采用经过修改的 BNF 语法，并辅以正则表达式和非形式化的文字约束。`mdbook-spec` 扩展会解析这些规则，并将其转换为可渲染的格式，包括铁路图。

代码块必须指定一个信息字符串，由单词 "grammar"、一个逗号和语法类别组成，如下所示：

~~~
```grammar,items
ProductionName -> SomeExpression
```
~~~

类别用于在附录的语法汇总页面中将相似的产生式归组。

<a id="grammar-syntax"></a>

## 语法定义的语法

语法定义语言的语法与[记法一章](../src/notation.md)中描述的记法十分接近，仅在渲染方式上略有差异。

带有 `@root` 标记的“根”产生式，是指未被任何其他产生式使用的产生式。

语法规范语言的形式化语法（使用其自身记法定义）如下：

```
Grammar -> Production+

BACKTICK -> U+0060

LF -> U+000A

Production -> `@root`? Name ` ->` Expression

Name -> <Alphanumeric or `_`>+

Expression -> Sequence (` `* `|` ` `* Sequence)*

Sequence -> (` `* AdornedExpr)+

AdornedExpr -> ExprRepeat Suffix? Footnote?

Suffix -> ` _` <not underscore, unless in backtick>* `_`

Footnote -> `[^` ~[`]` LF]+ `]`

ExprRepeat ->
      Expr1 `?`
    | Expr1 `*?`
    | Expr1 `*`
    | Expr1 `+?`
    | Expr1 `+`
    | Expr1 `{` Range? `..` Range? `}`

Range -> [0-9]+

Expr1 ->
      Unicode
    | NonTerminal
    | Break
    | Terminal
    | Charset
    | Prose
    | Group
    | NegativeExpression

Unicode -> `U+` [`A`-`Z` `0`-`9`]4..4

NonTerminal -> Name

Break -> LF ` `+

Terminal -> BACKTICK ~[LF]+ BACKTICK

Charset -> `[` (` `* Characters)+ ` `* `]`

Characters ->
      CharacterRange
    | CharacterTerminal
    | CharacterName

CharacterRange -> BACKTICK <any char> BACKTICK `-` BACKTICK <any char> BACKTICK

CharacterTerminal -> Terminal

CharacterName -> Name

Prose -> `<` ~[`>` LF]+ `>`

Group -> `(` ` `* Expression ` `* `)`

NegativeExpression -> `~` ( Charset | Terminal | NonTerminal )
```

整体格式是一系列由空行分隔的产生式。各类表达式如下：

| 表达式 | 示例 | 说明 |
|------------|---------|-------------|
| Unicode | U+0060 | 单个 Unicode 字符。 |
| NonTerminal | FunctionParameters | 按名称引用另一个产生式。 |
| Break | | 渲染器内部使用，用于识别换行和缩进。 |
| Terminal | \`example\` | 用反引号括起的精确字符序列。 |
| Charset | [ \`A\`-\`Z\` \`0\`-\`9\` \`_\` ] | 从一个字符集合中选择，集合成员以空格分隔。共有三种不同形式。 |
| CharacterRange | [ \`A\`-\`Z\` ] | 字符范围，其中每个字符都应以反引号括起。 |
| CharacterTerminal | [ \`x\` ] | 用反引号括起的单个字符。 |
| CharacterName | [ LF ] | 引用另一个产生式的非终结符。 |
| Prose | \<any ASCII character except CR\> | 用尖括号括起的英文描述，说明应匹配的内容（示例表示“除 CR 外的任意 ASCII 字符”）。 |
| Group | (\`,\` Parameter)+ | 对表达式分组以确定优先级，例如将重复运算符应用于由其他表达式组成的序列。 |
| NegativeExpression | ~[\` \` LF] | 匹配给定 Charset、Terminal 或 NonTerminal 以外的任何内容。 |
| Sequence | \`fn\` Name Parameters | 必须按顺序匹配的表达式序列。 |
| Alternation | Expr1 \| Expr2 | 匹配给定表达式中的一个，表达式之间以竖线字符（`\|`）分隔。 |
| Suffix | \_except \[LazyBooleanExpression\]\_ | 为前一个表达式附加下标说明。支持有限的 Markdown；应避免使用基本链接以外的标记。 |
| Footnote | \[^extern-safe\] | 附加脚注引用。脚注定义本身应以普通 Markdown 形式放在代码块之外。 |
| Optional | Expr? | 前一个表达式可选。 |
| Repeat | Expr* | 前一个表达式重复零次或多次。 |
| Repeat（非贪婪） | Expr*? | 前一个表达式以非贪婪方式重复零次或多次。 |
| RepeatPlus | Expr+ | 前一个表达式重复一次或多次。 |
| RepeatPlus（非贪婪） | Expr+? | 前一个表达式以非贪婪方式重复一次或多次。 |
| RepeatRange | Expr{2..4} | 前一个表达式在指定范围内重复。任一边界均可省略，其行为与 Rust 范围相同。 |

<a id="automatic-linking"></a>

## 自动链接

插件会在每个页面上为所有产生式名称自动添加 Markdown 链接定义。如需直接链接到某个产生式名称，请用方括号括起它，例如 `[ArrayExpression]`。

如果自动生成的规则链接与另一个锚点名称冲突，可使用 `grammar-` 前缀消除歧义，例如 `[Type][grammar-Type]`。在需要更清晰地表达意图时，也可以使用这种显式语法。
