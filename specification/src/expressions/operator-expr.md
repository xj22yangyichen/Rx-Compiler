# Operators

```grammar,expressions
OperatorExpression ->
      BorrowExpression
    | DereferenceExpression
    | NegationExpression
    | ArithmeticOrLogicalExpression
    | ComparisonExpression
    | LazyBooleanExpression
    | TypeCastExpression
    | AssignmentExpression
    | CompoundAssignmentExpression
```

## Arithmetic and bits

```grammar,expressions
NegationExpression ->
      `-` Expression
    | `!` Expression

ArithmeticOrLogicalExpression ->
      Expression `+` Expression
    | Expression `-` Expression
    | Expression `*` Expression
    | Expression `/` Expression
    | Expression `%` Expression
    | Expression `&` Expression
    | Expression `|` Expression
    | Expression `^` Expression
    | Expression `<<` Expression
    | Expression `>>` Expression
```

The arithmetic operators `+`, `-`, `*`, `/`, and `%` operate on compatible integer operands without implicit integer conversion. Unary minus requires a signed integer. Bitwise `&`, `|`, `^`, and `!` operate on integers; they also provide non-short-circuit boolean operations. Boolean `!` performs logical negation on `bool` operands.

Primitive arithmetic and bit operations support the scalar and reference operands in the following table. T denotes one supported scalar type, I denotes a supported signed integer type, and L/R denote independently chosen supported integer types.

| Operation | Operand types | Result |
| --- | --- | --- |
| Binary arithmetic `+`, `-`, `*`, `/`, `%` | Each operand is T or &T, with the same integer T on both sides | T |
| Binary bitwise `&`, <code>&#124;</code>, `^` | Each operand is T or &T, with the same integer or bool T on both sides | T |
| Shift `<<`, `>>` | Left is L or &L; right is R or &R | L |
| Unary `-` | I or &I | I |
| Unary `!` | T or &T, for integer or bool T | T |
| Arithmetic/bitwise compound assignment | A mutable place of primitive type T; right is T or &T for the corresponding operation | Unit |
| Shift compound assignment | A mutable place of primitive integer type L; right is R or &R | Unit |

Expected types do not propagate through binary or unary operators to their operands. Programs requiring that propagation for integer inference exhibit [undefined behavior](../types.md#coercion-sites-and-expected-types). For example, `let n: u32 = 1 + 2;` and `let n: isize = -1;` are undefined behavior; writing `1u32 + 2u32` and `-1isize` supplies the types explicitly.

Compound assignment likewise does not infer the right operand's integer type from the destination. For `n: u32`, `n += 1;` requires that inference and is undefined behavior; `n += 1u32;` is valid when `n` is mutable. All operand requirements in the table still apply.

<details>
<summary>Reference operand details</summary>

These arithmetic/bitwise reference variants accept one shared-reference layer, not arbitrary automatic dereferencing. For example, `&a + &b`, `-&a`, and `x += &y` are supported for compatible primitives; `&&a + &b`, `-&mut a`, and `x += &mut y` are not. A mutable-reference operand can be explicitly dereferenced to obtain the scalar value. A reference-valued assignment destination is not automatically dereferenced: use `*p += rhs` to update the primitive behind p.

The operations compute scalar results; they do not change a referent merely because an operand is a reference. Evaluating a reference operand produces its reference value; the scalar load needed by the operation occurs when that operation is performed, after its operands have been evaluated. Compound-assignment ordering is specified below.

The table defines the complete builtin operand combinations for these operations.

</details>

All supported integers have 32 bits. Runtime addition, subtraction, multiplication, and signed negation wrap using two's-complement arithmetic, as with Rust overflow checks disabled. Optimized and unoptimized code have the same behavior. LLVM overflow flags must not assert non-overflow unless the compiler has proved it for the particular operation.

Signed division truncates toward zero; a nonzero remainder has the sign of the dividend. Division or remainder operations where the divisor is zero, or where the signed minimum integer (`i32::MIN`) is divided or reduced modulo -1, are excluded from valid executions. No check or panic runtime is required.

Shifts accept integer operands whose types may differ. The result has the left operand's type. At runtime, shift amounts are masked to their least significant 5 bits. Signed right shift is arithmetic; unsigned right shift is logical. This masking also applies to constant-folded ordinary expressions.

## Comparison and logic

```grammar,expressions
ComparisonExpression ->
      Expression `==` Expression
    | Expression `!=` Expression
    | Expression `>` Expression
    | Expression `<` Expression
    | Expression `>=` Expression
    | Expression `<=` Expression

LazyBooleanExpression ->
      Expression `||` Expression
    | Expression `&&` Expression
```

Equality operators use the [PartialEq and equality rules](../builtin-traits.md#partialeq), including operand typing, implicit borrowing, and structural comparison. Equality between different source types is undefined behavior; see the [cross-type equality guarantee](../undefined-behavior.md#cross-type-equality).

Scalar ordering uses `<`, `<=`, `>`, `>=` on matching integer types and bool (false precedes true). Reference operands compare target values, not addresses. Their reference layers must have matching mutability at each depth and end in the same supported scalar type. One additional operand combination is supported: a left operand of type `&T` allows a right operand of type `&mut T`, adjusted to `&T`, with exactly the same referent type `T`. This operator-specific adjustment does not depend on an expected result type and does not rewrite inner reference layers.

For example, `&a < &b`, `&mut a < &mut b`, and `&a < &mut b` work for matching ordered scalars; `&mut a < &b` and `&&a < &&mut b` do not. There is neither automatic comparison between values and references (such as `a < &b`) nor automatic dereferencing through `Box` for ordering.

`&&` and `||` require bool values, return bool, and short-circuit from left to right. They have no reference-operand variants: `&true && true` is invalid. There is no integer truthiness. Chained comparisons such as `a < b < c` are not permitted directly; they require explicit parentheses and compatible intermediate types (e.g., `(a < b) < c`, where `a < b` evaluates to `bool`), or logical conjunction (`a < b && b < c`).

## Casts

```grammar,expressions
TypeCastExpression -> Expression `as` TypeNoBounds
```

`as` supports integer-to-integer casts and bool-to-integer casts. Since all integers are 32 bits, integer casts preserve the 32-bit pattern and interpret it in the destination signedness. False becomes 0 and true becomes 1. Integer-to-bool and reference-to-integer casts are not supported. The cast target does not supply an expected type to its operand: in `1 as u32`, the literal is `i32` and the cast produces `u32`.

### Cast parsing

<details>
<summary>Disambiguating generic arguments from operators</summary>

The [operator precedence table](../expressions.md#precedence) governs expression nesting. After a type-path segment in a cast, `<` begins [GenericArgs] rather than a comparison. A leading `<` from `<<` likewise enters type-argument parsing. Parenthesizing the cast makes the intended operation explicit: `(x as usize) < y` and `(x as usize) << y`. A parenthesized type already closes the type syntax, so `x as (usize) < y` and `x as (usize) << y` also parse as comparison and shift. Operators such as `<=`, `>`, `>>`, and `==` follow ordinary precedence after the cast type.

The parser checks the resulting syntax; name resolution checks its type arguments. Thus `x as usize<i32>` has a type-path syntax tree but is a compile error because the primitive usize takes no type arguments.

</details>

## Borrow, dereference, and assignment

```grammar,expressions
BorrowExpression ->
      (`&` | `&&`) Expression
    | (`&` | `&&`) `mut` Expression

DereferenceExpression -> `*` Expression

AssignmentExpression -> Expression `=` Expression

CompoundAssignmentExpression ->
      Expression `+=` Expression
    | Expression `-=` Expression
    | Expression `*=` Expression
    | Expression `/=` Expression
    | Expression `%=` Expression
    | Expression `&=` Expression
    | Expression `|=` Expression
    | Expression `^=` Expression
    | Expression `<<=` Expression
    | Expression `>>=` Expression
```

`&place` forms a shared reference; `&mut place` requires a mutable place. Applied to a value expression, borrowing materializes a temporary. `*reference` accesses its target, and `*box` accesses the owned `T` under the [Box rules](../heap.md#box-access-and-moves). Other types, including `Vec`, are not dereference operands. In prefix borrow position, `&&x` means `&(&x)`; infix `left && right` is short-circuit boolean and.

Each explicit `*` performs one dereference. Dereferencing `&T` gives shared
access; dereferencing `&mut T` gives mutable access unless reached through a
shared reference. An `&mut T` stored in an immutable local variable, struct, array,
or `Box` can still be dereferenced to modify its target. Once a shared reference is crossed along
the place's access path, further dereferences cannot restore mutable access.

All parameters below are immutable bindings. Each function can still modify
the integer through its stored mutable reference:

```rust,ignore
struct Holder<'a> {
    value: &'a mut i32,
}

fn through_local(value: &mut i32) {
    *value = 2; // one dereference reaches the integer
}

fn through_struct(holder: Holder<'_>) {
    *holder.value = 2; // modifies the integer, not the field holding the reference
}

fn through_array(values: [&mut i32; 1]) {
    *values[0] = 2; // indexes the reference, then dereferences it
}

fn through_box(value: Box<&mut i32>) {
    **value = 2; // first * accesses the Box content; second * follows &mut i32
}
```

The following functions fail to compile because their first dereference crosses
a shared reference, preventing the inner mutable reference from granting mutable access:

```rust,ignore
fn through_shared_reference(value: &&mut i32) {
    **value = 2; // compile error: the outer & gives only shared access
}

fn through_shared_box(value: &Box<&mut i32>) {
    ***value = 2; // compile error: & then Box then &mut still crosses shared access
}
```

`place = value` stores with the required copy/move behavior. Compound assignment applies the corresponding operation to the current scalar value at the destination. Both forms produce unit. The following order rules apply even if reference-operation lowering ultimately uses the same scalar instructions.

## Assignment destinations

An assignment destination is one mutable [place](../expressions.md#places-and-values). The assigned value must have a compatible type. A destination that fails these requirements is a compile error.

Assignments such as `s = other;` and `a = other_array;` assign an entire struct or array value into the destination place in a single operation.

## Evaluation order

| Expression form | Evaluation order |
| --- | --- |
| Unary scalar or shared-reference operation | Evaluate the operand, then perform the operation |
| Ordinary binary arithmetic, bitwise, shift, or comparison, including reference variants | Evaluate the left operand, then the right, then perform the operation |
| Lazy boolean `&&` / <code>&#124;&#124;</code> | Evaluate the left bool; evaluate the right only when required |
| Ordinary assignment `place = value` | Evaluate the right value, then the destination address, then store |
| Compound assignment with primitive scalar or shared-reference right operand | Evaluate the right operand, then the destination address, then perform the scalar read/update |

All supported compound assignments use the same right-first order, independently of whether the right operand is a scalar or a shared reference. Each operand expression and destination-address computation is evaluated once. The destination's old value is read after operand evaluation. A reference right operand is evaluated to its reference value first; its target is read when the scalar operation is performed. Borrow validity continues to be guaranteed by tests, not checked by a required borrow checker.

<details>
<summary>Compound-assignment order example</summary>

Under the course rules:

```rust,ignore
fn destination(p: &mut i32) -> &mut i32 {
    println_i32(2);
    p
}

fn right_value() -> i32 {
    println_i32(1);
    3
}

fn right_reference(p: &i32) -> &i32 {
    println_i32(1);
    p
}

fn main() {
    let mut x = 0;
    let y = 3;
    *destination(&mut x) += right_value();       // prints 1, then 2
    *destination(&mut x) += right_reference(&y); // prints 1, then 2
}
```

Official tests are independent of the relative evaluation order of the two operands of compound assignment. The example above illustrates the specified behavior, but evaluation order between the operands is not checked by the grading test suite.

</details>

Overwriting or dropping an existing value does not clear or zero its bytes in memory. Replacing a container does not require recursive destruction or immediate deallocation; its old heap storage may remain under the [heap cleanup rules](../heap.md#heap-cleanup).
