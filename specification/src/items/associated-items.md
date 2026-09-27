# Associated items

r[items.associated.syntax]
```grammar,items
AssociatedItem -> ConstantItem | Function
```

An inherent `impl` block may contain associated constants, associated functions, and methods.

Associated functions and methods are governed by the [function and receiver rules](functions.md#receivers). Associated items are referenced via a [PathInExpression], such as `Point::new` or `Self::LIMIT`. Method calls use [MethodCallExpression] and the [receiver lookup rules](../expressions/method-call-expr.md#method-lookup).
