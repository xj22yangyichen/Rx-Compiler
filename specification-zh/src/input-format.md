# 输入格式

一个源代码编译单元是包含 7 位 ASCII 源文本的单个文件。支持 LF 和 CRLF 行尾。Unicode 标识符、字节顺序标记和解释器指令不属于 Rx 语言子集。

[空白字符](whitespace.md)和[注释](comments.md)用于分隔词法单元。解析器根据 [Crate] 产生式处理由此得到的词法单元序列。
