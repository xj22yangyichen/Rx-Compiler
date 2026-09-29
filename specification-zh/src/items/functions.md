<a id="functions"></a>
# 函数

r[items.fn.syntax]
```grammar,items
Function ->
    `fn` IDENTIFIER GenericParams?
        `(` FunctionParameters? `)`
        FunctionReturnType? WhereClause?
        BlockExpression

FunctionParameters ->
      SelfParam `,`?
    | (SelfParam `,`)? FunctionParam (`,` FunctionParam)* `,`?

SelfParam -> ShorthandSelf

ShorthandSelf -> (`&` Lifetime?)? `mut`? `self`

FunctionParam -> IdentifierBinding `:` Type

FunctionReturnType -> `->` Type
```

函数定义由名称、可选的生命周期参数、形参列表、可选的返回类型、可选的生命周期约束和必需的函数体组成。顶层函数只接受普通形参，而固有 `impl` 块内的函数还可以包含接收者参数，如下文所述。

[GenericParams] 在函数名之后声明生命周期参数。[WhereClause] 在结果类型之后指定生命周期约束；省略结果类型注解时，则位于形参列表之后。参见[生命周期参数](generics.md)。

普通形参使用 [IdentifierBinding]，其后跟显式类型。函数签名独立于调用者确定形参和返回类型。接收者或最后一个普通形参之后允许有一个可选的尾随逗号。如果省略返回类型，则默认为单元类型 `()`。

普通形参绑定如果与可见的非限定 const 名称冲突，则属于未定义行为；参见[常量名称冲突保证](../undefined-behavior.md#constant-name-collisions)。

[名称查找规则](../names.md#scope-rules)允许前向调用和相互递归。函数名只能用作调用目标；将函数作为一等值使用会导致[未定义行为](../expressions/path-expr.md#function-values)。[调用表达式](../expressions/call-expr.md)规定实参求值和参数传递；[返回表达式](../expressions/return-expr.md)规定函数返回值。程序入口点在[程序入口](../undefined-behavior/builtin.md#program-entry)中定义。

```rust,ignore
fn add(left: i32, mut right: i32) -> i32 {
    right += left;
    right
}
```

<a id="receivers"></a>
## 接收者

在固有 impl 内，第一个参数可以是 [SelfParam]。其形式如下：

| 形式 | 接收者 |
| --- | --- |
| `self` | 按值接收者 |
| `mut self` | 可变的按值接收者 |
| `&self` 或 `&'a self` | 共享引用 |
| `&mut self` 或 `&'a mut self` | 可变引用 |

带有接收者的函数是方法；没有接收者的函数是关联函数。所有后续形参都必须遵循 [FunctionParam] 语法。接收者的生命周期遵循[生命周期有效性规则](../references.md#lifetime-validity)，按值接收者遵循[复制/移动语义](../builtin-traits.md#copy)。
