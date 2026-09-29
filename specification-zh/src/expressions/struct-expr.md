<a id="struct-expressions"></a>
# 结构体表达式

r[expr.struct.syntax]
```grammar,expressions
StructExpression -> PathInExpression `{` StructExprFields? `}`

StructExprFields -> StructExprField (`,` StructExprField)* `,`?

StructExprField -> IDENTIFIER `:` Expression
```

结构体表达式由类型路径及其后用花括号界定的 `field: expression` 初始化器列表组成。该路径必须在类型命名空间中解析为已声明的具名字段结构体。支持的形式包括带有允许的生命周期实参或省略形式的结构体名称（例如 `View::<'a> { value: x }` 或 `View { value: x }`），以及 `impl` 块内的 `Self`。[路径解析](../paths.md#path-resolution)确定该具名结构体，而生命周期有效性由测试套件保证。

每个声明的字段必须恰好出现一次，并使用类型与该字段声明类型兼容的表达式初始化。缺失字段、重复字段、未知字段以及不兼容的值都会导致编译错误。字段初始化器按照[普通求值顺序](../expressions.md#evaluation-order)以源代码顺序求值。在条件边界处，[条件解析规则](if-expr.md)区分结构体初始化器的左花括号与控制流主体块的左花括号。

```rust,ignore
struct Point { x: i32, y: i32 }
fn main() {
    let point = Point { y: 2, x: 1 };
    println_i32(point.x);
}
```
