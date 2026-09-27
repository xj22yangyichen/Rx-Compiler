# Functions

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

A function definition consists of a name, optional lifetime parameters, a parameter list, an optional return type, optional lifetime bounds, and a required body. Top-level functions accept only ordinary parameters, whereas functions within inherent `impl` blocks may also include a receiver parameter, as described below.

[GenericParams] declares lifetime parameters after the function name. [WhereClause] states lifetime bounds after the result type, or after the parameter list when the result annotation is omitted. See [lifetime parameters](generics.md).

Ordinary parameters use [IdentifierBinding] followed by an explicit type. Function signatures determine parameter and return types independently of their callers. An optional trailing comma is permitted after a receiver or the last ordinary parameter. If the return type is omitted, it defaults to the unit type `()`.

An ordinary parameter binding that collides with a visible unqualified const name is undefined behavior; see the [constant-name collision guarantee](../undefined-behavior.md#constant-name-collisions).

The [name lookup rules](../names.md#scope-rules) permit forward calls and mutual recursion. Function names may only serve as call targets; treating a function as a first-class value results in [undefined behavior](../expressions/path-expr.md#function-values). [Call expressions](../expressions/call-expr.md) specify argument evaluation and parameter passing; [return expressions](../expressions/return-expr.md) specify function return values. The program entry point is defined in [Program entry](../undefined-behavior/builtin.md#program-entry).

```rust,ignore
fn add(left: i32, mut right: i32) -> i32 {
    right += left;
    right
}
```

## Receivers

Within an inherent impl, the first parameter may be [SelfParam]. Its forms are:

| Form | Receiver |
| --- | --- |
| `self` | By-value receiver |
| `mut self` | Mutable by-value receiver |
| `&self` or `&'a self` | Shared reference |
| `&mut self` or `&'a mut self` | Mutable reference |

A function with a receiver is a method; a function without a receiver is an
associated function. Any subsequent parameters must follow the [FunctionParam] syntax. Receiver lifetimes
follow the [lifetime validity rules](../references.md#lifetime-validity), and
by-value receivers follow [copy/move semantics](../builtin-traits.md#copy).
