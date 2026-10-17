<a id="unit-type"></a>
# 单元类型

```grammar,types
UnitType -> `(` `)`
```

单元类型写作 `()`，其唯一的值由 [UnitExpression] 描述。

其有效的数据使用以及函数/控制流结果例外由[零大小数据规则](../undefined-behavior.md#zero-sized-data)定义。

```rust,ignore
fn do_nothing() -> () {
    // 返回类型省略或为 () 的函数返回 ()
}

();                              // 单元字面量
```
