# Input format

A source compilation unit is a single file containing 7-bit ASCII source text. LF and CRLF line endings are supported. Unicode identifiers, byte-order marks, and interpreter directives are excluded from the Rx language subset.

[Whitespace](whitespace.md) and [comments](comments.md) separate tokens. The parser consumes the resulting sequence of tokens according to the [Crate] production.
