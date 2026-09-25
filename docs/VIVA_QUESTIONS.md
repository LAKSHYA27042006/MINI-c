# MiniC Compiler: Comprehensive Viva Voce Questions & Answers
## Compiler Design Laboratory Examination & Project Defense Guide

This document contains 45 high-yield, technically rigorous viva questions and detailed answers based directly on the actual implementation of the **MiniC Compiler**.

---

### **Section 1: Compiler Fundamentals & Architecture**

#### 1. What is a compiler?
**Answer:** A compiler is a language translator that converts computer code written in a high-level source language into another target language (such as machine code, bytecode, or an intermediate representation) without altering the program's intended meaning. It analyzes the entire source text statically before execution.

#### 2. What are the classical phases of a compiler?
**Answer:**
1. **Lexical Analysis (Scanner)**: Converts character stream into tokens.
2. **Syntax Analysis (Parser)**: Validates syntax and generates parse tree / AST.
3. **Semantic Analysis (Type Checker)**: Verifies static semantics, types, and scope rules.
4. **Intermediate Code Generation (ICG)**: Translates AST into a machine-independent intermediate representation (like Three-Address Code).
5. **Code Optimization**: Improves execution speed, memory usage, or code size.
6. **Code Generation**: Emits target machine code or assembly.
7. **Symbol Table & Error Handling**: Span across all phases.

#### 3. Why did you choose to build a compiler for "MiniC" rather than full ANSI C?
**Answer:** Full C contains preprocessor directives, pointers, arbitrary type casts, struct/union alignments, function pointers, and platform-specific ABIs, which would require tens of thousands of lines of boilerplate code. MiniC captures the fundamental constructs of imperative programming—primitive types, expressions, operator precedence, conditional branching, loops, and block scopes—enabling us to demonstrate core compiler theory cleanly and rigorously within an academic timeframe.

#### 4. Why did you implement the compiler in standard C (C11)?
**Answer:** C provides direct control over memory management, explicit data structures (structs, unions, pointers), deterministic execution, and zero external dependency overhead. Implementing the compiler in C reinforces low-level systems programming concepts and mirrors how classical industrial compilers (GCC, early Clang) are engineered.

#### 5. Why does this compiler stop at Three-Address Code (TAC) instead of emitting x86 or ARM assembly?
**Answer:** Generating machine code requires architecture-specific register allocation algorithms (like graph coloring), instruction scheduling, stack frame management, and ABI calling conventions. Stopping at TAC cleanly isolates machine-independent compilation and optimization concepts from target-dependent hardware idiosyncrasies.

---

### **Section 2: Lexical Analysis & Tokens**

#### 6. What is lexical analysis?
**Answer:** Lexical analysis is the first phase of a compiler. It reads the source code as an input stream of raw characters and groups them into meaningful atomic sequences known as lexemes, mapping each lexeme to a typed `Token` while discarding whitespace and comments.

#### 7. What is the difference between a token, a pattern, and a lexeme?
**Answer:**
- **Token**: An abstract category or terminal symbol (e.g., `TOKEN_KW_IF`, `TOKEN_INT_LITERAL`).
- **Lexeme**: The actual concrete sequence of source characters matching the token pattern (e.g., `"if"`, `"42"`).
- **Pattern**: The formal rule (such as a regular expression) describing the valid character sequences that form a token (e.g., `[0-9]+` for integers).

#### 8. What is the difference between a keyword and an identifier?
**Answer:** A keyword is a reserved word defined by the language specification (e.g., `int`, `while`) that has a fixed grammatical meaning and cannot be used as a variable name. An identifier is a user-defined name assigned to variables or entities. In MiniC, the lexer scans words matching `[a-zA-Z_][a-zA-Z0-9_]*` and compares them against a keyword table; if a match is found, a keyword token is returned; otherwise, `TOKEN_IDENTIFIER` is emitted.

#### 9. How does your lexer handle single-line and multi-line comments?
**Answer:** When the scanner encounters `/`, it inspects the next character. If it is `/`, it advances until `\n` or EOF. If it is `*`, it saves the start position and advances until `*/`. If EOF is reached before `*/`, a lexical error is reported indicating an unterminated multi-line comment with the exact starting line and column.

#### 10. How does your lexer detect and report lexical errors?
**Answer:** Any character that does not match whitespace, comments, identifiers, numeric literals, or recognized operator symbols (e.g., `@`, `$`, `#`) triggers `error_report()` with error type `ERR_LEXICAL`, specifying the exact line number, column number, and ASCII character value. The scanner then advances past the invalid character to continue tokenizing the remainder of the file.

---

### **Section 3: Parsing, Grammars, and AST**

#### 11. What is parsing?
**Answer:** Parsing (syntax analysis) is the phase that verifies whether the token stream produced by the lexer conforms to the formal context-free grammar of the language, constructing a hierarchical structural representation (AST).

#### 12. What is Recursive Descent Parsing, and why did you choose it?
**Answer:** Recursive descent is a top-down parsing technique where each non-terminal in the grammar is implemented as a dedicated C function. We chose it because it is intuitive, modular, highly readable, easy to debug without complex external state tables (unlike LALR parsers), and enables fine-grained syntax error reporting.

#### 13. How did you implement operator precedence and associativity in your parser?
**Answer:** We implemented precedence climbing through hierarchical grammar tiers:
- `parse_expression()` handles low-precedence additive operators (`+`, `-`).
- `parse_term()` handles higher-precedence multiplicative operators (`*`, `/`, `%`).
- `parse_factor()` handles atomic factors (literals, identifiers, parenthesized expressions, and unary operators).
Because `parse_expression()` calls `parse_term()`, multiplication and division naturally bind tighter than addition and subtraction.

#### 14. What is an Abstract Syntax Tree (AST), and how does it differ from a Parse Tree (Concrete Syntax Tree)?
**Answer:** A Parse Tree contains every grammatical production rule, including punctuation tokens like semicolons, commas, and explicit parentheses. An AST strips away syntactic sugar and punctuation, retaining only the semantic structure: interior nodes represent operators/statements, and leaf nodes represent operands/literals.

#### 15. How is an AST node represented in your C code?
**Answer:** As a dynamically allocated `ASTNode` struct containing an `ASTNodeType` enum, coordinate metadata (`line`, `column`), an `inferred_type` field, and a tagged union storing node-specific payloads (e.g., left and right subtrees for binary expressions, condition and body pointers for while loops).

#### 16. How does your parser handle syntax errors without crashing?
**Answer:** When an expected token is not matched, `parser_error()` records a descriptive syntax error with the current token's line, column, expected token, and found token. The parser then enters `panic_mode` and executes `synchronize()`, discarding tokens until a statement boundary (such as `;` or `{`) is encountered, allowing syntax analysis to resume and catch further errors.

---

### **Section 4: Symbol Table & Semantic Analysis**

#### 17. What is a symbol table, and what information does it store in MiniC?
**Answer:** A symbol table is a compile-time record store mapping identifier names to their attributes. In MiniC, each `Symbol` entry stores:
- Variable name (`char *`)
- Data type (`int` or `float`)
- Scope depth level and scope name (`global`, `block-1`, etc.)
- Source declaration line and column
- Initialization flag (`is_initialized`)
- Constant flag and known compile-time value (`const_val`).

#### 18. How is lexical scoping implemented in your symbol table?
**Answer:** Scopes are organized in a scope stack linked via parent pointers (`parent`). When entering a `{`, `symbol_table_enter_scope()` creates a new `Scope` linked to the current scope. When exiting `}`, `symbol_table_exit_scope()` points back to the parent. Lookup operations search from the innermost active scope outwards up to the global scope.

#### 19. How is variable shadowing supported?
**Answer:** A nested block may declare a variable with the same name as a variable in an outer scope. Because `symbol_table_lookup()` traverses from the innermost scope outwards, the inner symbol is found first, naturally shadowing the outer symbol until the block exits.

#### 20. What is the difference between `lookup_current` and `lookup`?
**Answer:** `lookup_current()` checks only the current local scope; this is used during variable declaration to detect duplicate declarations in the same scope. `lookup()` traverses the entire ancestor chain up to the global scope to resolve variable references in expressions.

#### 21. What semantic checks are performed by your semantic analyzer?
**Answer:**
1. **Declaration checking**: Every variable used in assignments or expressions must be declared.
2. **Duplicate declaration checking**: Variables cannot be declared more than once within the same scope.
3. **Type compatibility**: Modulo `%` requires integer operands; assignment of `float` to `int` generates a precision-loss warning.
4. **Uninitialized variable warnings**: Emits warnings if a variable is read before assignment.

---

### **Section 5: Three-Address Code (TAC)**

#### 22. What is Three-Address Code (TAC)?
**Answer:** TAC is an intermediate representation where every instruction has at most one operator and at most three operand addresses: two source operands and one destination.

#### 23. Why use Three-Address Code instead of operating directly on the AST?
**Answer:** An AST is a deeply nested tree structure, making linear transformations and machine-level optimizations cumbersome. TAC linearizes control flow into jumps and labels, flattens complex expressions into atomic assignments, and introduces explicit virtual registers (temporaries), closely resembling the execution flow of actual hardware.

#### 24. What are temporary variables and labels in TAC?
**Answer:**
- **Temporaries (`t1`, `t2`, ...)**: Compiler-generated virtual registers used to hold intermediate computation results of sub-expressions.
- **Labels (`L1`, `L2`, ...)**: Named markers in the instruction stream that serve as branch targets for conditional and unconditional jumps.

#### 25. How is a `while` loop translated into Three-Address Code?
**Answer:**
1. Emit start label `L_start:`.
2. Evaluate condition into temporary `t_cond`.
3. Emit conditional jump `ifFalse t_cond goto L_end`.
4. Generate code for the loop body.
5. Emit unconditional jump `goto L_start`.
6. Emit end label `L_end:`.

#### 26. How is an `if-else` statement translated into TAC?
**Answer:**
1. Evaluate condition into temporary `t_cond`.
2. Emit `ifFalse t_cond goto L_else`.
3. Generate then-branch code.
4. Emit `goto L_end`.
5. Emit label `L_else:`.
6. Generate else-branch code.
7. Emit label `L_end:`.

---

### **Section 6: Peephole Optimization**

#### 27. What is peephole optimization?
**Answer:** Peephole optimization is a localized optimization technique that slides a small window over a sequence of intermediate instructions and replaces recognizable patterns with shorter, faster, or simpler instructions.

#### 28. What is Constant Folding, and what safety checks are required?
**Answer:** Constant folding computes operations on compile-time known constants at compile time (e.g., `t1 = 3 + 4` $\to$ `t1 = 7`). Safety check: In division and modulo (`/`, `%`), the second operand must be checked for zero; if zero, the compiler must not fold to prevent runtime crashes.

#### 29. What is Algebraic Simplification? Give three examples implemented in MiniC.
**Answer:** Algebraic simplification replaces instructions with mathematically equivalent ones based on arithmetic identities:
1. Addition identity: `x = y + 0` $\to$ `x = y`
2. Multiplication identity: `x = y * 1` $\to$ `x = y`
3. Annihilator property: `x = y * 0` $\to$ `x = 0`

#### 30. What is Constant Propagation, and what are its scope boundaries?
**Answer:** Constant propagation substitutes variables assigned constant values into subsequent expressions within a basic block (e.g., `x = 10; y = x + 5` $\to$ `y = 10 + 5`). In MiniC, propagation is confined to straight-line code: whenever a label, branch, or jump is encountered, all propagated constant bindings are cleared to prevent unsafe assumptions across merge points.

#### 31. What is Redundant Temporary Elimination?
**Answer:** When a temporary is generated to hold a binary result and immediately copied to a user variable (`t1 = a + b; x = t1`), if `t1` is never read again, the two instructions are collapsed directly into `x = a + b`, removing the intermediate temporary and copy instruction.

#### 32. What is Dead Code Elimination?
**Answer:** The removal of instructions that calculate values that are never used. MiniC implements:
1. Removal of pure temporary definitions that have zero downstream uses.
2. Removal of dead variable stores where variable `x` is overwritten before any intermediate read within the same basic block.

#### 33. What is Unreachable Code Elimination?
**Answer:** The removal of instructions following an unconditional `goto` instruction up to the next target `TAC_LABEL`, as execution can never reach those instructions sequentially.

#### 34. How does the optimizer ensure all cascading optimizations are caught?
**Answer:** The optimizer runs iteratively in a multi-pass loop (up to 10 passes). A pass executes constant propagation, constant folding, algebraic simplification, temporary elimination, dead code elimination, and unreachable code elimination. If any transformation occurs (`changed = true`), another pass is executed until a fixed point (no further changes) is reached.

#### 35. How are the optimization report metrics calculated?
**Answer:** Statistics counters are incremented dynamically during each transformation pass. At the end of optimization:
$$\text{Instructions Removed} = \text{Original Count} - \text{Optimized Count}$$
$$\text{Reduction Percentage} = \frac{\text{Instructions Removed}}{\text{Original Count}} \times 100\%$$
Zero metrics are hard-coded.

---

### **Section 7: Engineering, Memory Management, & Testing**

#### 36. How do you prevent memory leaks in your C implementation?
**Answer:** Every dynamically allocated data structure has an explicit destructor function:
- `ast_free()` recursively frees AST nodes, strings, and statement arrays.
- `token_array_free()` frees token arrays and lexeme buffers.
- `symbol_table_free()` frees all symbols, strings, and scopes across all recorded scopes.
- `tac_program_free()` frees all doubly linked TAC instructions.
- `error_reporter_free()` frees all error records.

#### 37. What compiler flags were used to ensure code quality?
**Answer:** We compiled with `-std=c11 -Wall -Wextra -Wpedantic -O2`, ensuring strict conformance to ISO C11 and eliminating all warnings.

#### 38. How is the test suite organized, and how do you ensure tests are not faked?
**Answer:** `tests/test_runner.c` is a standalone executable containing 54 programmatic assertions that call the real compiler library functions in-process, verifying return codes, AST structures, symbol tables, and generated TAC for both valid code and error conditions.

#### 39. What was the most technically challenging part of the project?
**Answer:** Coordinating the multi-pass optimizer without introducing invalid transformations across basic block boundaries, specifically ensuring that constant propagation and redundant temporary elimination correctly respect variable re-assignments and control-flow jumps.

#### 40. How does MiniC handle floating-point values alongside integers?
**Answer:** Floating-point numbers are detected by the scanner via the presence of a decimal point. In the AST, semantic analysis infers mixed-type arithmetic (widening `int` to `float`), while the optimizer correctly executes constant folding for both integer and double-precision float operations.

#### 41. What is the difference between a top-down and a bottom-up parser?
**Answer:** A top-down parser (like recursive descent or LL) starts at the grammar root symbol (`program`) and attempts to derive the token stream by expanding productions downwards. A bottom-up parser (like LR, LALR, or shift-reduce) starts with the input tokens and repeatedly reduces them back to the start symbol.

#### 42. What happens if an undeclared variable is used in MiniC?
**Answer:** Semantic analysis detects that `symbol_table_lookup()` returns `NULL`, logs `ERR_SEMANTIC` ("Variable 'x' is not declared"), halts code generation, and exits with code 1.

#### 43. Why is Three-Address Code called "Three-Address"?
**Answer:** Because each standard computational instruction references at most three memory addresses or registers: one destination and two sources (e.g., $A = B \text{ op } C$).

#### 44. How would you extend MiniC to support functions?
**Answer:**
1. Grammar: Add function declaration syntax `type IDENTIFIER '(' params ')' block`.
2. AST: Add `NODE_FUNC_DECL` and `NODE_CALL` nodes.
3. Symbol Table: Add function signatures (return type, parameter types).
4. TAC: Add `TAC_PARAM`, `TAC_CALL`, `TAC_RETURN`, and function prologue/epilogue labels.

#### 45. What would change if you replaced your TAC generator with LLVM IR?
**Answer:** We would emit LLVM's Static Single Assignment (SSA) intermediate representation using the LLVM C API or text emitter (`%1 = add nsw i32 %a, %b`). We would then gain access to LLVM's industrial optimization pipeline (`opt`) and native code generators for x86, ARM, and WebAssembly.
