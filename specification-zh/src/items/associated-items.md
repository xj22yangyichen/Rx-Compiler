<a id="associated-items"></a>
# 关联条目

r[items.associated.syntax]
```grammar,items
AssociatedItem -> ConstantItem | Function
```

固有 `impl` 块可以包含关联常量、关联函数和方法。

关联函数和方法受[函数与接收者规则](functions.md#receivers)约束。关联条目通过 [PathInExpression] 引用，例如 `Point::new` 或 `Self::LIMIT`。方法调用使用 [MethodCallExpression]，并遵循[接收者查找规则](../expressions/method-call-expr.md#method-lookup)。
