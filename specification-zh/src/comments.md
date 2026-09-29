r[comments]
# 注释

r[comments.syntax]
```grammar,lexer
@root COMMENT -> LINE_COMMENT | BLOCK_COMMENT

LINE_COMMENT -> `//` ~LF*

BLOCK_COMMENT -> `/*` ( BLOCK_COMMENT | BLOCK_CHAR )* `*/`

BLOCK_CHAR -> CHAR _not at the start of `/*` or `*/`_
```

注释支持行注释（`//`）和可嵌套块注释（`/* ... */`）两种形式，并起空白字符的作用。行注释在 LF 字符之前或文件末尾结束；CRLF 序列中的 CR 被视为要忽略的注释文本的一部分。文件末尾的空行注释是有效的。

在块注释的每个位置，`/*` 开启嵌套注释，`*/` 关闭当前注释。除此之外，每次读取一个字符作为 [BLOCK_CHAR]。分隔符优先于普通内容，每个开始分隔符都必须有匹配的结束分隔符。未终止的注释无效。

```rust,ignore
// A line comment.
/* outer /* inner */ outer again */
/***/
/* a trailing slash / */
fn main() {}
```

<a id="documentation-comments"></a>
## 文档注释

文档注释，包括外部形式（`///`、`/** ... */`）和内部形式（`//!`、`/*! ... */`），根据[测试保证](undefined-behavior.md#test-guarantees)属于未定义行为。
