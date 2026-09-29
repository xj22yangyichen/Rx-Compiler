# 语句与绑定

```grammar,statements
Statement -> `;` | LetStatement | ExpressionStatement
```

## 标识符绑定

```grammar,statements
LetStatement -> `let` IdentifierBinding (`:` Type)? `=` Expression `;`

IdentifierBinding -> `mut`? IDENTIFIER
```

`let` 语句将一个标识符绑定到初始化表达式的求值结果。指定 `mut` 会使绑定可变。初始化表达式可以产生任何受支持的值，包括借用表达式形成的引用。

绑定的可见性和遮蔽遵循[名称](names.md#scope-rules)中的规则。`let` 绑定若与可见的非限定常量名称冲突，则属于未定义行为；参见[常量名称冲突保证](undefined-behavior.md#constant-name-collisions)。

可选的类型标注和初始化表达式遵循[局部类型推断](types.md#inference)及[强制转换规则](types.md#conversions-and-coercions)。对该绑定的后续赋值遵循[赋值目标](expressions/operator-expr.md#assignment-destinations)的规则。

## 表达式语句

```grammar,statements
ExpressionStatement ->
      ExpressionWithoutBlock `;`
    | ExpressionWithBlock `;`?
```

表达式语句对表达式求值并丢弃结果，同时保留其副作用。空的 `;` 语句没有效果。

[ExpressionWithoutBlock] 要求分号。[ExpressionWithBlock]（`{ ... }`、`if`、`while` 或 `loop`）可以省略分号。省略分号时，表达式的求值类型必须与单元类型 `()` 兼容，或者表达式必须发散。块的最终表达式遵循[尾表达式规则](expressions/block-expr.md)。

```rust,ignore
fn show(flag: bool) {
    if flag { print_i32(1); }    // unit result: semicolon optional
    if flag { 1 } else { 2 };   // integer result: semicolon discards it
}
```

<a id="statement-boundary"></a>
## 语句边界

解析表达式语句时，具有外层块形式的表达式会立即终止语句，而不会贪婪地继续吸收后续中缀运算符。在初始化器或其他值表达式上下文中，表达式则正常继续。括号可以强制进入普通表达式上下文。

附着的 `else` 或 `else if` 子句仍属于其 `if` 表达式，后缀字段访问或方法调用也可以直接跟在已完成的块表达式之后。其他表达式语句延续到终止分号；或者，外层块可以用紧邻结束 `}` 之前的尾表达式结束。例如，只要 `make()` 返回带有 `value` 字段的结构体，`{ make() }.value;` 就是有效的字段访问表达式语句。

```rust,ignore
let value = if true { 10 } else { 20 } - 1; // initializer is the whole subtraction
if true {} else {} -1;                      // if statement, then -1 expression statement
(if true { 10 } else { 20 }) - 1;           // one expression statement
```

符合规范的编译器必须实现这些语句边界，并遵守上面定义的语法。
