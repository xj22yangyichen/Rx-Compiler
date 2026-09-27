r[comments]
# Comments

r[comments.syntax]
```grammar,lexer
@root COMMENT -> LINE_COMMENT | BLOCK_COMMENT

LINE_COMMENT -> `//` ~LF*

BLOCK_COMMENT -> `/*` ( BLOCK_COMMENT | BLOCK_CHAR )* `*/`

BLOCK_CHAR -> CHAR _not at the start of `/*` or `*/`_
```

Comments support both line (`//`) and nestable block (`/* ... */`) forms, and act as whitespace. A line comment ends before an LF character or at the end of the file; a CR in a CRLF sequence is treated as part of the ignored comment text. An empty line comment at the end of the file is valid.

At each position in a block comment, `/*` opens a nested comment and `*/` closes the current one. Otherwise, one character is consumed as [BLOCK_CHAR]. Delimiters take priority over ordinary content, and each opening delimiter requires a matching closing delimiter. Unterminated comments are invalid.

```rust,ignore
// A line comment.
/* outer /* inner */ outer again */
/***/
/* a trailing slash / */
fn main() {}
```

## Documentation comments

Documentation comments, including outer (`///`, `/** ... */`) and inner (`//!`, `/*! ... */`) forms, exhibit undefined behavior under the [test guarantees](undefined-behavior.md#test-guarantees).
