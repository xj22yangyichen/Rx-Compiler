# Constant contexts

Constant contexts are const-item initializers, array type lengths, and array-repeat lengths. Each uses the restricted forms defined below, including references to constant items.

## Allowed forms

```grammar,constants
ConstValue -> INTEGER_LITERAL | `true` | `false` | ConstantPath | `-` Magnitude | `(` ConstValue `)`

Magnitude -> INTEGER_LITERAL | ConstantPath | `(` Magnitude `)`

ConstantPath -> PathInExpression
```

`ConstantPath` must resolve to a constant item, either directly or through an associated-constant path such as `LIMIT`, `Config::LIMIT`, or `Self::LIMIT`. A path to a local, function, or other value is a static form error. In this grammar, `Magnitude` denotes an integer literal or constant path without a leading minus, rather than an unsigned target type. Negative forms require a signed integer type. These forms permit `123`, `LIMIT`, `-LIMIT`, `(-1)`, `-(1)`, `((true))`, and valid suffixes and radices.

Each constant context contains an integer or boolean literal, a constant-item reference, an optionally parenthesized negative integer or constant, or parentheses around another allowed form. Any other expression is a static form error.

```rust,ignore
const COUNT: usize = 64;
const COPY: usize = COUNT;
const SENTINEL: i32 = (-1);
const ENABLED: bool = (true);

fn main() {
    let data: [i32; COUNT] = [0; COUNT];
    let n: usize = COPY; // ordinary expressions may also read a constant
}
```

Constant paths are evaluated recursively through their referenced constant
items. A constant item may be declared before or after the item that uses it.
The dependency graph must be acyclic: a self-reference or an indirect cycle
such as `A -> B -> A` is a compile error. The compiler must detect a cycle
before attempting to evaluate the involved constants.

## Typing and range

A constant path has the declared type and value
of its target item. Const-item initializers have the declared expected type;
array lengths are constant values and have expected type `usize`.

Literal typing and signed-minimum cases follow [integer literal rules](expressions/literal-expr.md#integer-typing-and-range). Out-of-range literals are undefined behavior under the [integer literal range guarantee](undefined-behavior.md#integer-literal-range). Ordinary type and restricted-form errors remain compile errors.

The [propagation exclusion](types.md#coercion-sites-and-expected-types) also applies here: `const N: isize = -1;` is undefined behavior because it needs the expected type to pass through unary minus. Write `-1isize` instead. `const N: i32 = -1;` is valid because the literal `-1` defaults to `i32`.

The element expression in `[expr; 4]` is an ordinary expression governed by [array repetition](expressions/array-expr.md#array-expressions). Only its length is a constant context.

Constant propagation and folding of ordinary runtime expressions remain permitted optimizations. They do not turn those expressions into const contexts or change their arithmetic semantics.
