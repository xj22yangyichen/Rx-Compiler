<a id="path-expressions"></a>
# 路径表达式

r[expr.path.syntax]
```grammar,expressions
PathExpression -> PathInExpression
```

路径表达式解析非限定的值名称、`self` 或关联条目（如 `Point::LIMIT`）。路径形式与命名空间规则在[路径](../paths.md)和[名称](../names.md)中规定。

函数路径用作调用目标。在有效程序中，`Box` 和 `Vec` 构造函数是对 [PathExpression] 的调用，其容器段携带 [GenericArgs]；省略类型实参会导致[未定义行为](../heap.md#builtin-signatures)。

<a id="function-values"></a>
## 函数值

根据[测试保证](../undefined-behavior.md#test-guarantees)，将函数用作值（例如 `let f = helper;`）属于未定义行为。这适用于普通函数、关联函数和方法，以及内建条目。不要求提供函数值表示或诊断。调用函数及使用其结果遵循普通的[调用规则](call-expr.md)。

```rust,ignore
fn helper() -> i32 { 42 }

let result = helper();          // 有效：调用函数路径
// let f = helper;              // 未定义行为：将函数条目作为一等值使用
```
