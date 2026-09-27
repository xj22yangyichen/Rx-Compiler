r[lex.token]
# Tokens

r[lex.token.syntax]
```grammar,lexer
@root Token ->
      IDENTIFIER_OR_KEYWORD
    | INTEGER_LITERAL
    | LIFETIME_TOKEN
    | PUNCTUATION
```

Tokens are identifiers or keywords, integer literals, lifetime tokens, and punctuation. Keywords, including boolean literals (`true` and `false`), share the lexical character patterns of identifiers, but their syntactic roles are determined by keyword rules and the [LiteralExpression] production.

Tokenization selects the longest token subject to the numeric and lifetime boundaries below. The parser may consume prefixes of combined punctuation according to [contextual punctuation](grammar.md#contextual-punctuation).

## Integer literals

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

Integer literals use decimal digits or a binary, octal, or hexadecimal prefix, with optional underscore separators. A non-decimal literal must contain at least one digit after the prefix. The four integer suffixes specify the corresponding integer type.

| Form | Examples |
| --- | --- |
| Decimal | `123`, `1_234`, `123_i32` |
| Binary | `0b1010`, `0b____1`, `0b10_u32` |
| Octal | `0o77`, `0o7_usize` |
| Hexadecimal | `0xff`, `0xAB_CD`, `0xff_isize` |

The lexer consumes the complete numeric token using maximal munch. The lexer must not recover from an invalid suffix or invalid radix digit by splitting the input into a valid integer token followed by trailing tokens; spellings such as `123i32foo`, `123bad`, `0b102`, and `0x` must be rejected as invalid tokens. A supplied Rust lexer may retain a general suffix and reject unsupported suffixes at the language-subset boundary.

There is no token-length or numeric-magnitude limit. Compilers must preserve the raw digits and suffix without requiring the literal's magnitude to fit into a host-platform integer type. Type determination and range validity follow [literal expressions](expressions/literal-expr.md#integer-typing-and-range).

A literal outside its determined type's range is undefined behavior; see the [integer literal range guarantee](undefined-behavior.md#integer-literal-range).

A leading minus sign (`-`) is tokenized as a separate operator, not as part of an integer literal. Because `f`, `3`, and `2` are valid hexadecimal digits, `0x01_f32` is parsed as a single hexadecimal integer literal whose value is `0x01f32` with no suffix, rather than as a floating-point literal.

## Lifetimes

r[lex.token.life.syntax]
```grammar,lexer
LIFETIME_TOKEN -> `'` IDENTIFIER_OR_KEYWORD

LIFETIME_OR_LABEL -> `'` NON_KEYWORD_IDENTIFIER
```

A lifetime token consists of an apostrophe immediately followed by its name, with no intervening whitespace or comment. The lexer consumes the complete name. Examples include `'a`, `'data`, `'static`, and `'_`. A lifetime token must not be followed by a closing apostrophe; spellings such as `'a'` represent character literals, which are not supported in Rx.

[LIFETIME_OR_LABEL] supplies ordinary named lifetimes to [Lifetime]. The special spellings `'static` and `'_` have their own alternatives there. Lifetime tokens occur in reference types, parameter declarations, path arguments, and bounds. Although the lexer production `LIFETIME_OR_LABEL` matches loop label tokens, Rx control-flow productions ([Loops](expressions/loop-expr.md)) do not support labeled loops or jumps.

## Punctuation

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

Comments take priority over `/` punctuation. Combined operators follow the longest-token rule; their contextual splits are specified in [Parser conventions](grammar.md#contextual-punctuation).

Floating-point numbers are not supported in Rx; numeric tokens containing fractional or exponent parts (such as `1.0` or `1e5`) must be rejected as lexical errors rather than split into integer and punctuation tokens. Delimiters (parentheses, brackets, and braces) must always occur in matched pairs. The `#` character is used exclusively to introduce outer derive attributes.
