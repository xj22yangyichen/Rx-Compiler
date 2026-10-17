<a id="unit-expressions"></a>
# 单元表达式

```grammar,expressions
UnitExpression -> `(` `)`
```

表达式 `()` 构造单元值。

单元值 `()` 是函数或控制流表达式完成执行但不产生返回值时的结果。根据[零大小数据保证](../undefined-behavior.md#zero-sized-data)，将单元值存入变量、作为实参传递或嵌入聚合数据或堆数据中会导致未定义行为。

```rust,ignore
fn log_status() {
    // 隐式求值为 ()
}

log_status();                    // 求值为单元值 () 并丢弃该值
```
