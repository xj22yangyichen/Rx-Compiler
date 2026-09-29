r[lex.whitespace]
# 空白字符

```grammar,lexer
@root WHITESPACE -> ( SPACE | TAB | LF | CR LF )+
```

空白分隔符包括空格、水平制表符、LF 和 CRLF。

空白字符用于分隔词法单元，除此之外没有语义意义。将一种空白分隔符替换为另一种受支持的分隔符不会改变词法单元序列。[注释](comments.md)也起分隔符作用。不能在标识符内部、整数字面量内部，或注释分隔符的两个字符之间插入空白字符，例如 `//` 或 `/*`。
