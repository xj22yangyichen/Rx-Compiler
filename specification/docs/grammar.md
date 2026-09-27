# Grammar

The reference grammar is written in Markdown code blocks using a modified BNF syntax augmented with regular expressions and informal prose constraints. The `mdbook-spec` extension parses these rules and converts them to a renderable format, including railroad diagrams.

The code block must specify an info string consisting of the word "grammar", a comma, and the grammar category, as shown below:

~~~
```grammar,items
ProductionName -> SomeExpression
```
~~~

The category is used to group similar productions on the grammar summary page in the appendix.

## Grammar syntax

The syntax of the grammar definition language closely mirrors the notation described in the [Notation chapter](../src/notation.md), with minor differences in rendering.

A "root" production, marked with `@root`, is one that is not used in any other production.

The formal syntax of the grammar specification language (defined in its own notation) is:

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

The general format is a series of productions separated by blank lines. The expressions are:

| Expression | Example | Description |
|------------|---------|-------------|
| Unicode | U+0060 | A single Unicode character. |
| NonTerminal | FunctionParameters | A reference to another production by name. |
| Break | | This is used internally by the renderer to detect line breaks and indentation. |
| Terminal | \`example\` | This is a sequence of exact characters, enclosed in backticks. |
| Charset | [ \`A\`-\`Z\` \`0\`-\`9\` \`_\` ] | A choice from a set of characters, space-separated. There are three different forms. |
| CharacterRange | [ \`A\`-\`Z\` ] | A range of characters, where each character should be in backticks. |
| CharacterTerminal | [ \`x\` ] | A single character, surrounded by backticks. |
| CharacterName | [ LF ] | A nonterminal, referring to another production. |
| Prose | \<any ASCII character except CR\> | This is an English description of what should be matched, enclosed in angle brackets. |
| Group | (\`,\` Parameter)+ | This groups an expression for the purpose of precedence, such as applying a repetition operator to a sequence of other expressions. |
| NegativeExpression | ~[\` \` LF] | Matches anything except the given Charset, Terminal, or Nonterminal. |
| Sequence | \`fn\` Name Parameters | A sequence of expressions, where they must match in order. |
| Alternation | Expr1 \| Expr2 | Matches only one of the given expressions, separated by the vertical bar character (`\|`). |
| Suffix | \_except \[LazyBooleanExpression\]\_  | Appends a subscript description to the previous expression. Limited Markdown is supported; avoid markup beyond basic links. |
| Footnote | \[^extern-safe\] | Attaches a footnote reference. The footnote definition itself should be placed outside of the code block as normal Markdown. |
| Optional | Expr? | The preceding expression is optional. |
| Repeat | Expr* | The preceding expression is repeated 0 or more times. |
| Repeat (non-greedy) | Expr*? | The preceding expression is repeated 0 or more times without being greedy. |
| RepeatPlus | Expr+ | The preceding expression is repeated 1 or more times. |
| RepeatPlus (non-greedy) | Expr+? | The preceding expression is repeated 1 or more times without being greedy. |
| RepeatRange | Expr{2..4} | The preceding expression is repeated within the specified range. Either bound can be excluded, which works just like Rust ranges. |

## Automatic linking

The plugin automatically adds markdown link definitions for all the production names on every page. To link directly to a production name, enclose it in square brackets, like `[ArrayExpression]`.

If an automatically generated rule link collides with another anchor name, disambiguate it using the `grammar-` prefix, such as `[Type][grammar-Type]`. This explicit syntax may also be used whenever greater clarity is desired.
