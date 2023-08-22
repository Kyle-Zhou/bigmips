## Compiler

Translating a basic (heavily restricted) subset of C++ into MIPS assembly

#### Analysis
- ***Scanning***: tokenizing input program
- ***Parsing***: takes tokens from scanning stage & creates a parse tree via SLR(1) bottom up parsing
- ***Context Sensitive (Semantic) Analysis***: checking for semantic errors + building the symbol table

#### Synthesis
- ***Code Generation***: reconstructing symbol table and building a new procedure table to help generate the MIPS assembly code which ultimately implements the behaviour specified by the original input program.

#### Assembly
- converts the MIPS assembly into machine code