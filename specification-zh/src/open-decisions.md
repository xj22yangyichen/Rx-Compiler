# 发布验证

本工作规范定义 2026 年课程的语言和执行契约。发布前需要针对选定的前端和执行环境完成以下集成检查。

## 剩余发布工作

| Area | Validation | Reference |
| --- | --- | --- |
| Parser integration | Parse the documented type, item, expression, and statement forms, including lifetime declarations/arguments/bounds, nested containers, explicit turbofish, contextual punctuation, and condition/statement boundaries | [Grammar summary](grammar-summary.md), [Parser conventions](grammar.md) |
| Runtime integration | Execute Box/Vec construction, growth, non-Copy remove, nested and recursive Clone/equality, and fresh heap state for each execution with the published runtime and pinned REIMU commit | [Heap](heap.md), [Backend contract](backend.md) |

分配器 ABI、参考 `Vec` 增长策略、内存和栈设置以及累计堆预算由[后端资源保证](backend.md#resource-guarantees)规定。源代码所有权和引用有效性遵循[堆清理规则](heap.md#heap-cleanup)。

## 规范状态

目录定义完整的课程语言。构建文档可以验证文档结构和内部链接，而上面的集成任务根据语言语法验证解析器和运行时。课程考核、提交和评分另行记录。
