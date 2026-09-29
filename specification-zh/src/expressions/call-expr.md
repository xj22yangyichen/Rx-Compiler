<a id="call-expressions"></a>
# 调用表达式

r[expr.call.syntax]
```grammar,expressions
CallExpression -> Expression `(` CallParams? `)`

CallParams -> Expression ( `,` Expression )* `,`?
```

调用实参遵循[普通求值顺序](../expressions.md#evaluation-order)，并按照声明的形参类型和[复制/移动规则](../builtin-traits.md#copy)传递。被调用者必须解析为已声明的函数、关联函数或内建条目。最后一个实参后可以有一个可选的尾随逗号。

关联方法也可以通过 `Type::method(receiver, ...)` 调用，此时接收者作为普通的第一个实参提供。此形式使用普通的实参强制转换，而不是方法调用的自动借用。`Box` 和 `Vec` 的构造函数调用使用相同的调用语法；省略容器路径段上的显式类型实参属于[未定义行为](../heap.md#builtin-signatures)。

```rust,ignore
fn add(a: i32, b: i32) -> i32 { a + b }

struct Counter { val: i32 }
impl Counter {
    fn new() -> Counter { Counter { val: 0 } }
    fn inc(&mut self) { self.val += 1; }
}

let sum = add(1, 2);
let mut c = Counter::new();
Counter::inc(&mut c);           // 不进行自动借用的完全限定调用语法
```
