<a id="constant-items"></a>
# 常量条目

r[items.const.syntax]
```grammar,items
ConstantItem -> `const` IDENTIFIER `:` Type `=` ConstValue `;`
```

常量声明指定名称、显式类型和初始化器。初始化器表达式必须求值得到符合[常量上下文规则](../const_eval.md)的 [ConstValue]。常量初始化器可以引用其他常量条目，但须遵循相同规则，且依赖关系必须无环。在普通表达式中引用常量时，会求值得到它定义的值。

常量在顶层声明，或作为固有 impl 中的关联条目声明。[名称查找规则](../names.md#scope-rules)允许前向引用，因此可以在常量的文本声明之前使用它。

```rust,ignore
const LIMIT: i32 = 100;
const NEGATIVE: i32 = (-1);
```
