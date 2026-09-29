# Trait 与属性

Rx 支持四种[内建 Trait](builtin-traits.md)的 derive 属性：`Copy`、`Clone`、`PartialEq` 和 `Eq`。不支持用户定义的 Trait、Trait 约束和显式 Trait 实现。

<a id="attribute-grammar"></a>
## 属性语法

```grammar,attributes
OuterAttribute -> `#` `[` DeriveAttribute `]`

DeriveAttribute -> `derive` `(` (DeriveName (`,` DeriveName)* `,`?)? `)`

DeriveName -> `Copy` | `Clone` | `PartialEq` | `Eq`
```

这是完整的属性语法。顶层具名字段 [Struct] 前可以有零个或多个外部属性（[OuterAttribute]）。不支持其他构造上的属性、内部属性（`#![...]`）以及其他属性名或 derive 名称。

derive 列表可以为空，也可以有尾随逗号。结构体上的所有 derive 属性共同组成同一组请求的 Trait；在同一个属性中或同一结构体的多个属性之间重复指定 Trait 属于编译时错误。每个请求的 Trait 都必须满足其[要求](builtin-traits.md#trait-capability-summary)。

```rust,ignore
#[derive(Clone)]
#[derive(Copy, PartialEq, Eq,)]
struct Point { x: i32, y: i32 }
```
