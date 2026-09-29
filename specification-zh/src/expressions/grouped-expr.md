<a id="grouped-expressions"></a>
# 分组表达式

r[expr.paren.syntax]
```grammar,expressions
GroupedExpression -> `(` Expression `)`
```

括号表达式具有其内部操作数的值和类型，并保留其位置属性。括号可以在语句边界处强制采用表达式上下文，也可以消除条件中的结构体构造或 `<`、`<<` 之前的类型转换的歧义。

```rust,ignore
let mut x = 5;
(x) = 10;                     // 加括号的位置仍是可赋值的位置
let y = (1 + 2) * 3;          // 分组覆盖默认的运算符优先级
```

空括号 `()` 构成 [UnitExpression] 所描述的单元表达式。
