# Notation

## Grammar

The following notations are used by the *Lexer* and *Syntax* grammar snippets:

| Notation          | Examples                      | Meaning                                   |
|-------------------|-------------------------------|-------------------------------------------|
| CAPITAL           | IDENTIFIER, INTEGER_LITERAL        | A token produced by the lexer             |
| _ItalicCamelCase_ | _LetStatement_, _Item_        | A syntactical production                  |
| `string`          | `x`, `while`, `*`             | The exact character(s)                    |
| x<sup>?</sup>     | `mut`<sup>?</sup>             | An optional item                          |
| x<sup>\*</sup>    | _OuterAttribute_<sup>\*</sup> | 0 or more of x                            |
| x<sup>+</sup>     |  _Statement_<sup>+</sup>     | 1 or more of x                            |
| x<sup>a..b</sup>  | HEX_DIGIT<sup>1..6</sup>      | a to b repetitions of x                   |
| Rule1 Rule2       | `fn` _Name_ _Parameters_      | Sequence of rules in order                |
| \|                | `i32` \| `u32`, Block \| Item  | Alternation (matches one of the alternatives) |
| `[ ]`              | `[b B]`                       | Any of the characters listed              |
| `[ - ]`            | `[a-z]`                       | Any of the characters in the range        |
| `~[ ]`             | `~[b B]`                      | Any characters, except those listed       |
| ~`string`         | ~`\n`, ~`*/`                  | Any characters, except this sequence      |
| ( )               | (`,` _Parameter_)<sup>?</sup> | Groups items                              |
| U+xxxx            | U+0060                        | A single Unicode character                |
| \<text\>          | \<any ASCII char except CR\>  | An English description of what should be matched |
| Rule <sub>suffix</sub> | IDENTIFIER_OR_KEYWORD <sub>_except `crate`_</sub> | A modification to the previous rule |

Sequencing has higher precedence than `|` alternation.

<details>
<summary>String-table productions</summary>

## String-table productions

Certain rules, including [unary operators], [binary operators], and [keywords], are defined as tables of literal strings. In syntax productions, literal string references implicitly match the corresponding token produced by lexical analysis. See [Tokens][tokens].

</details>

## Grammar visualizations

Grammar productions appear in the chapters that define them. Lexer rules use uppercase names; syntax rules use mixed-case names. The button below each grammar block shows its [syntax diagram], where rectangles represent nonterminals and rounded rectangles represent terminals.

[syntax diagram]: https://en.wikipedia.org/wiki/Syntax_diagram

## Common productions

The following are common definitions used in the grammar.

r[input.syntax]
```grammar,lexer
CHAR -> <a 7-bit ASCII source character>

SPACE -> U+0020

TAB -> U+0009

LF -> U+000A

CR -> U+000D
```

[binary operators]: expressions/operator-expr.md#arithmetic-and-bits
[keywords]: keywords.md
[tokens]: tokens.md
[unary operators]: expressions/operator-expr.md#borrow-dereference-and-assignment
