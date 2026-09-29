# Crates and source files

```grammar,items
@root Crate -> Item*
```

An Rx program consists of a single source compilation unit containing a sequence of top-level items. Its functions, structs, constants, and inherent impls share the namespaces described in [Names](names.md).

The top-level [Item] alternatives are `use` declarations, functions, named-field structs, constants, and inherent `impl` blocks. [Use declarations](items/use-declarations.md) exist solely for Rust compatibility and may be discarded after parsing; they do not introduce any names into Rx namespaces. Items cannot be declared in expression blocks. Program entry follows [the main contract](undefined-behavior/builtin.md#program-entry).
