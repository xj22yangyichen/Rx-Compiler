# Method-call expressions

r[expr.method.syntax]
```grammar,expressions
MethodCallExpression -> Expression `.` PathExprSegment `(` CallParams? `)`
```

A method call gives a receiver expression, a method identifier, and a parenthesized argument list. An optional trailing comma follows the last argument.

[PathExprSegment] supplies the shared path-segment syntax. Methods use their identifier with optional lifetime arguments where Rust permits them. [Path resolution](../paths.md#path-resolution) specifies these arguments and the lifetime validity guarantee. Type arguments on a method segment are compile errors.

## Method lookup

The receiver type must be known before lookup. Method calls find the receiver adjustments as follows:

1. Start with the receiver expression's type and repeatedly dereference references and `Box`, recording each type in order. `Vec` adds no dereference step.
2. Immediately after each recorded type `T`, insert `&T` and `&mut T` as candidate receiver types.
3. Visit candidate receiver types in that order. For each candidate:
   - If multiple distinct methods match that candidate (such as an inherent method and a derived builtin `Clone` method sharing the same receiver type), the call exhibits undefined behavior.
   - If exactly one method matches that candidate, select it and terminate candidate search. Include inherent methods, available builtin `Clone` methods, and the specified array and container methods.
   - If no method matches that candidate, proceed to the next candidate in the list.
4. If no candidate in the list matches any method, compilation fails with a compile error. Argument types, expected result types, and receiver mutability do not filter whether a method matches.
5. For the selected method, apply the dereferences and any borrow for that candidate; an existing mutable-reference receiver may reborrow under the [ordinary reference rules](../types.md#mutable-reference-reborrowing). Then check receiver mutability and ordinary arguments. If any check fails, compilation fails with a compile error, subject to the separate ownership and lifetime test guarantees.

Tests exclude competing methods on the same candidate; the compiler need not detect them or implement Rust's inherent-over-trait priorities. Duplicate inherent declarations remain [name errors](../names.md#name-collisions); methods on unrelated types outside the receiver's candidate list do not compete.

<details>
<summary>Receiver and clone examples</summary>

For `Box<S>`, the candidate receiver types are `Box<S>`, `&Box<S>`, `&mut Box<S>`, `S`, `&S`, and `&mut S`. For a unique method on `S` taking `&self`, calling it on the box dereferences to `S` and borrows that place. An `&mut self` method requires mutable access to the reached `S` place under the [place rules](../expressions.md#places-and-values).

If `S` derives `Clone` and has no inherent `clone` method, calling `s.clone()` on `s: S` matches candidate `&S` and returns `S`. Likewise, calling `r.clone()` on `r: &S` matches candidate `&S` directly without checking `&&S`, returning `S`.

However, if `S` derives `Clone` and also declares an inherent method named `clone` (such as `fn clone(&self)`), calling `s.clone()` on `s: S` results in undefined behavior because both the inherent method and the derived trait method match candidate `&S`.

Similarly, for `b: Box<i32>`, candidate `&Box<i32>` matches `Box::<i32>::clone` before `i32`'s candidates are reached, cloning the container into a new `Box<i32>`.

</details>

An explicit `Type::method(receiver, ...)` call selects that type's associated operation and uses [ordinary argument coercions](call-expr.md), not receiver lookup or autoref. If an inherent method and a builtin method on that exact type share the name, the inherent method is selected. See [Builtin traits](../builtin-traits.md#clone) and [Heap](../heap.md) for available builtin operations.
