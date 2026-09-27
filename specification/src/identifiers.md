r[ident]
# Identifiers

r[ident.syntax]
```grammar,lexer
IDENTIFIER_OR_KEYWORD -> ( ASCII_ALPHA | `_` ) ASCII_CONTINUE*

ASCII_ALPHA -> [`a`-`z` `A`-`Z`]

ASCII_CONTINUE -> ASCII_ALPHA | DEC_DIGIT | `_`

NON_KEYWORD_IDENTIFIER -> IDENTIFIER_OR_KEYWORD _except `_` and a [strict](keywords.md#strict-keywords) or [reserved](keywords.md#reserved-keywords) keyword_

IDENTIFIER -> NON_KEYWORD_IDENTIFIER
```

Identifiers use ASCII letters, digits, and underscores as specified above. They are case-sensitive and have no fixed length limit. `_value`, `_1`, and `__` are identifiers; `_` alone is punctuation and cannot be a binding or assignment destination.

`self` and `Self` are keywords with the semantics specified in [Names](names.md). In contrast, built-in names such as `i32`, `Vec`, and `Clone` are lexically standard identifiers; their restrictions are enforced by namespace scoping rules rather than dedicated lexer keyword rules.
