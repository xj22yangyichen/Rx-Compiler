# Publication validation

This working specification defines the language and execution contract for the
2026 course. Publication requires the following integration checks against the
selected frontend and execution environment.

## Remaining publication work

| Area | Validation | Reference |
| --- | --- | --- |
| Parser integration | Parse the documented type, item, expression, and statement forms, including lifetime declarations/arguments/bounds, nested containers, explicit turbofish, contextual punctuation, and condition/statement boundaries | [Grammar summary](grammar-summary.md), [Parser conventions](grammar.md) |
| Runtime integration | Execute Box/Vec construction, growth, non-Copy remove, nested and recursive Clone/equality, and fresh heap state for each execution with the published runtime and pinned REIMU commit | [Heap](heap.md), [Backend contract](backend.md) |

The allocator ABI, reference `Vec` growth policy, memory and stack settings, and cumulative heap budget are specified by the [backend resource guarantees](backend.md#resource-guarantees). Source ownership and reference validity follow the [heap cleanup rules](heap.md#heap-cleanup).

## Specification status

The table of contents defines the complete course language. Building the documentation verifies document structure and internal links, while the integration tasks above validate the parser and runtime against the language grammar. Course assessment, submissions, and scoring are recorded separately.
