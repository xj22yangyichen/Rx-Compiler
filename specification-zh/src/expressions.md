# Expressions and evaluation

```grammar,expressions
Expression -> ExpressionWithoutBlock | ExpressionWithBlock

ExpressionWithoutBlock ->
      LiteralExpression
    | PathExpression
    | OperatorExpression
    | GroupedExpression
    | ArrayExpression
    | IndexExpression
    | UnitExpression
    | StructExpression
    | CallExpression
    | MethodCallExpression
    | FieldExpression
    | ContinueExpression
    | BreakExpression
    | ReturnExpression

ExpressionWithBlock ->
      BlockExpression
    | LoopExpression
    | IfExpression
```

Expressions form integer, boolean, and unit values; resolve names and associated paths; construct Box/Vec values, arrays, and named-field structs; access fields and indexed elements; call functions and methods; apply operators; and control execution through blocks, if/while/loop, and return/break/continue. Parentheses group expressions. Heap operations are specified in [Box and Vec](heap.md).

## Evaluation order

Ordinary operands evaluate left to right before the enclosing operation. This includes call arguments, array elements, struct field initializers, comparisons, arithmetic, and indexing. Struct fields evaluate in source order rather than declaration or layout order.

Specialized rules for [assignment](expressions/operator-expr.md#evaluation-order), [short-circuit logic](expressions/operator-expr.md#comparison-and-logic), [array repetition](expressions/array-expr.md#array-expressions), and control flow specify which operands are evaluated and in what order.

Optimizations must preserve observable effects, including I/O and writes through references.

## Places and values

A place denotes storage: a variable, dereference, field, indexed array or `Vec` element, or parenthesized place. Reading a place copies or moves its value according to its type. Assignment requires a mutable place, and borrowing evaluates to a reference containing the place's address.

A mutable local can be assigned. Dereferencing `&mut T` yields a mutable place
unless reached through a shared reference; dereferencing `&T` does not.

The following blocks are independent examples. A binding declared without
`mut` can hold an `&mut T` and modify its target, but cannot be redirected to
another target:

```rust,ignore
let mut x = 1;
let mut y = 2;
let p = &mut x;
*p = 3;        // valid: changes x, not the binding p
p = &mut y;    // compile error: p is not a mutable binding
```

Making the binding mutable permits both operations:

```rust,ignore
let mut x = 1;
let mut y = 2;
let mut p = &mut x;
*p = 3;        // valid: changes x
p = &mut y;    // valid: redirects p to y
*p = 4;        // valid: changes y
```

A shared reference does not permit modifying its target, even if that target
was declared mutable:

```rust,ignore
let mut x = 1;
let p = &x;
*p = 2;        // compile error: p gives shared access to x
```

Additionally, mutable access cannot be recovered through a shared reference to an `&mut T`:

```rust,ignore
let mut x = 1;
let p = &mut x;
let shared = &p;
**shared = 2;  // compile error: reaching x crosses the shared reference
```

### Temporary places

A local variable such as `values` names existing storage. A value expression such as
`make_array()` produces a value without naming a storage location. When an
operation needs a place for that value, the value is held in temporary storage.
This is called *materializing a temporary*. It applies to field and index
bases, explicitly borrowed values, and method receivers that need a borrow.

The value expression is evaluated once each time the enclosing expression is
evaluated. That result is used for the field, element, borrow, or method call;
obtaining its address must not evaluate it again. For example, indexing
`make_array()[0]` calls `make_array()` once, and any side effects of that call
occur once.

A temporary can be used mutably without a named `let mut` binding. With a
function `make_array() -> [i32; 2]`, these operations are valid:

```rust,ignore
make_array()[0] = 7;          // writes an element of the temporary array
Vec::<i32>::new().push(5);    // borrows the temporary vector as &mut self
let p = &mut 3;              // borrows a temporary integer
*p = 4;                     // modifies that integer
```

The same rule permits `make().field = value` for any returned struct containing that field.
Existing places still follow their own mutability rules: using an immutable
local or a shared reference as a base does not create a mutable copy of it.
Assignment still requires a place expression; `1 = 2` and `1 += 2` are invalid.
Blocks produce values with the ordinary copy/move semantics.

<details>
<summary>Temporary lifetime (optional background)</summary>

A temporary normally remains valid through its
containing statement. Certain `let` initializers extend its storage duration
to the end of the enclosing block. In this example, the explicit borrow in the initializer
extends the lifetime of the returned array:

```rust,ignore
fn make_array() -> [i32; 2] {
    println_i32(1);
    [10, 20]
}

fn main() {
    let r = &mut make_array()[0];
    *r = 99;
    println_i32(*r);
}
```

This prints `1`, then `99`, each on its own line. The call runs once, its
returned array occupies temporary storage, and `r` refers to its first element.
That array remains valid until the end of `main`'s block.

Returning a reference from an ordinary function or method call does not by
itself extend an argument temporary's lifetime. For example, using the
`make_array` function above:

```rust,ignore
fn first(values: &mut [i32; 2]) -> &mut i32 {
    &mut values[0]
}

fn example() {
    let r = first(&mut make_array()); // the temporary array ends at this semicolon
    *r = 99; // excluded lifetime violation: r would refer to expired storage
}
```

This last example is outside the [lifetime validity guarantee](undefined-behavior.md#lifetime-validity);
the compiler need not diagnose it. A conforming implementation may retain
temporary storage until function return, or optimize it away when observable
behavior and valid references are preserved, as described in
[References](references.md#storage-and-implementation). Retaining storage does
not make an otherwise invalid source reference valid.

</details>

## Precedence

The following groups run from strongest to weakest. Operators in a row share precedence.

| Group | Associativity |
| --- | --- |
| Paths, literals, grouping | Primary forms |
| Field and method access, calls, indexing | Postfix |
| Unary `-`, `!`, `*`, `&`, `&mut` | Prefix |
| `as` | Left |
| `*`, `/`, `%` | Left |
| `+`, `-` | Left |
| `<<`, `>>` | Left |
| `&` | Left |
| `^` | Left |
| <code>&#124;</code> | Left |
| `==`, `!=`, `<`, `<=`, `>`, `>=` | Cannot be chained without parentheses |
| `&&` | Left |
| <code>&#124;&#124;</code> | Left |
| `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `^=`, <code>&#124;=</code>, `<<=`, `>>=` | Right |
| `return`, `break` with a value | Consume the following expression |

Block/statement disambiguation additionally follows [Statements](statements.md#statement-boundary).
