r[ident]
# 标识符

r[ident.syntax]
```grammar,lexer
IDENTIFIER_OR_KEYWORD -> ( ASCII_ALPHA | `_` ) ASCII_CONTINUE*

ASCII_ALPHA -> [`a`-`z` `A`-`Z`]

ASCII_CONTINUE -> ASCII_ALPHA | DEC_DIGIT | `_`

NON_KEYWORD_IDENTIFIER -> IDENTIFIER_OR_KEYWORD _except `_` and a [strict](keywords.md#strict-keywords) or [reserved](keywords.md#reserved-keywords) keyword_

IDENTIFIER -> NON_KEYWORD_IDENTIFIER
```

标识符使用上面规定的 ASCII 字母、数字和下划线。标识符区分大小写，没有固定的长度限制。`_value`、`_1` 和 `__` 是标识符；单独的 `_` 是标点符号，不能作为绑定或赋值目标。

`self` 和 `Self` 是关键字，其语义见[名称](names.md)。相比之下，`i32`、`Vec` 和 `Clone` 等内建名称在词法上是普通标识符；它们的限制由命名空间作用域规则强制执行，而不是由专门的词法分析器关键字规则执行。
