# Call expressions

r[expr.call.syntax]
```grammar,expressions
CallExpression -> Expression `(` CallParams? `)`

CallParams -> Expression ( `,` Expression )* `,`?
```

Call arguments follow the [ordinary evaluation order](../expressions.md#evaluation-order) and are passed according to the declared parameter types and [copy/move rules](../builtin-traits.md#copy). The callee must resolve to a declared function, associated function, or builtin. An optional trailing comma follows the last argument.

An associated method may also be called as `Type::method(receiver, ...)`, with its receiver supplied as an ordinary first argument. That form uses ordinary argument coercions rather than method-call autoref. `Box` and `Vec` constructor calls use the same call syntax; omitting explicit type arguments on the container path segment is [undefined behavior](../heap.md#builtin-signatures).

```rust,ignore
fn add(a: i32, b: i32) -> i32 { a + b }

struct Counter { val: i32 }
impl Counter {
    fn new() -> Counter { Counter { val: 0 } }
    fn inc(&mut self) { self.val += 1; }
}

let sum = add(1, 2);
let mut c = Counter::new();
Counter::inc(&mut c);           // fully qualified call syntax without autoref
```
