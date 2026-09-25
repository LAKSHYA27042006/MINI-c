# MiniC Compiler: Project Presentation Deck Outline
## 15-Slide Presentation Structure & Speaker Script

---

### **Slide 1: Title Slide**
- **Title**: Mini-Compiler with Intermediate Code Generation and Peephole Optimization
- **Subtitle**: Implementation of a Complete Front-to-Middle Compiler Pipeline in Standard C11
- **Course**: Compiler Design Laboratory Individual Project
- **Key Focus**: Lexical Analysis, Recursive Descent Parsing, Scoped Symbol Tables, Three-Address Code, and Multi-Pass Peephole Optimization
- **Speaker Notes**: "Good morning respected evaluators. Today I am presenting my compiler design laboratory project: the MiniC Compiler. This system implements a full compiler pipeline from source code to optimized intermediate representation, completely built from scratch in C11."

---

### **Slide 2: Problem Statement**
- Full-scale production compilers (GCC, LLVM) encompass millions of lines of code.
- Heavy architectural complexity (ABIs, registers, machine assembly, preprocessor macros) obscures fundamental compiler concepts.
- Need: A transparent, rigorous, and modular compiler that demonstrates lexical analysis, parsing, semantic checking, and intermediate code optimization without black-box third-party generators.
- **Speaker Notes**: "Industrial compilers are too complex for academic transparency. Our goal is to isolate and visibly demonstrate the core theoretical phases of a compiler in clean, standard C."

---

### **Slide 3: Motivation**
- Build an end-to-end compiler from first principles using standard C libraries (`stdio.h`, `stdlib.h`, etc.).
- Avoid automatic parser generators (No Lex/Yacc, Flex/Bison).
- Explore the mechanics of Abstract Syntax Trees and Scoped Symbol Tables.
- Implement an authentic Three-Address Code (TAC) representation and quantify the impact of peephole optimization.
- **Speaker Notes**: "By implementing every phase manually, we gain direct insight into how programming languages are scanned, parsed, checked, translated, and optimized."

---

### **Slide 4: Objectives**
1. Hand-craft a deterministic lexical scanner with coordinate tracking.
2. Develop a predictive Recursive Descent Parser enforcing operator precedence.
3. Build an in-memory Abstract Syntax Tree (AST) with tree-walking visualization.
4. Implement a scope-stack Symbol Table supporting nested blocks and shadowing.
5. Generate structured Three-Address Code with explicit temporaries and control labels.
6. Design a multi-pass Peephole Optimizer supporting 6 transformation passes.
7. Dynamically calculate optimization metrics and build an automated test suite.
- **Speaker Notes**: "Our objectives are measurable: from 100% manual lexing and parsing to dynamic calculation of TAC instruction reductions."

---

### **Slide 5: MiniC Language Features**
- **Data Types**: `int` and `float` with automatic type inference and widening.
- **Declarations & Initializations**: `int x = 3 + 4;`, `float pi = 3.14;`
- **Arithmetic Operators**: `+`, `-`, `*`, `/`, `%` with standard precedence (*, /, % over +, -).
- **Relational Operators**: `<`, `>`, `<=`, `>=`, `==`, `!=`
- **Control Flow**: `if`, `if-else`, and `while` loops.
- **Scoping**: Lexical blocks `{ ... }` with local symbol scopes.
- **Comments**: Single-line (`//`) and multi-line (`/* ... */`).
- **Speaker Notes**: "MiniC captures all essential imperative programming primitives while omitting non-essential overhead like pointers and structs."

---

### **Slide 6: Compiler Architecture & Pipeline**
- Visual diagram showing:
  `Source Code -> Lexer -> Token Stream -> Parser -> AST -> Semantic Analyzer <-> Symbol Table -> TAC Generator -> Raw TAC -> Peephole Optimizer -> Optimized TAC -> Report`
- Strictly modular C architecture with decoupled `.h` and `.c` components.
- **Speaker Notes**: "This flowchart represents our compiler architecture. Information flows cleanly from stage to stage, culminating in optimized TAC."

---

### **Slide 7: Lexical and Syntax Analysis**
- **Lexer**:
  - Scans character by character, tracks line and column.
  - Recognizes keywords, identifiers, numbers, and operators.
  - Gracefully detects invalid characters (e.g., `@`).
- **Parser**:
  - Recursive Descent Parser implementing LL(1) grammar.
  - Precedence climbing: `parse_expression` $\to$ `parse_term` $\to$ `parse_factor`.
  - Panic-mode error recovery with synchronization.
- **Speaker Notes**: "Our parser guarantees that expressions like `a + b * c` parse with multiplication as a subtree under addition, respecting algebraic order of operations."

---

### **Slide 8: AST and Semantic Analysis**
- **Abstract Syntax Tree**:
  - Node structures with tagged unions.
  - Strips syntactic punctuation; retains semantic intent.
  - Visual hierarchical tree printer.
- **Semantic Analyzer & Scoped Symbol Table**:
  - Enforces declaration before use.
  - Detects duplicate declarations in the same scope.
  - Resolves nested scope shadowing (`global` vs `block-1`).
  - Validates operator compatibility (e.g. `%` disallowed on floats).
- **Speaker Notes**: "The symbol table maintains a scope stack. When a block is entered, a new scope is pushed; upon exit, the scope is popped, perfectly preserving lexical scoping."

---

### **Slide 9: Three-Address Code (TAC)**
- Linearized Intermediate Representation.
- Instructions have at most one operator and three address fields.
- **Structure**:
  - Temporaries: `t1`, `t2`, `t3`, ...
  - Labels: `L1`, `L2`, `L3`, ...
  - Control flow: `ifFalse <cond> goto <label>`, `goto <label>`.
- Preserves program semantics independently of machine architecture.
- **Speaker Notes**: "Three-Address Code linearizes the AST into discrete, atomic instructions, providing the ideal substrate for machine-independent optimization."

---

### **Slide 10: Peephole Optimization Passes**
- Multi-pass sliding window transformations until convergence (fixed point):
  1. **Constant Folding**: Compile-time arithmetic on constants (`3 + 4` $\to$ `7`).
  2. **Algebraic Simplification**: Algebraic identities (`x + 0` $\to$ `x`, `x * 1` $\to$ `x`, `x * 0` $\to$ `0`).
  3. **Constant Propagation**: Propagating known values within basic blocks.
  4. **Redundant Temporary Elimination**: Inlining single-use temporaries (`t1 = a + b; x = t1` $\to$ `x = a + b`).
  5. **Dead Code Elimination**: Removing overwritten stores and unused temporaries.
  6. **Unreachable Code Elimination**: Pruning dead code after unconditional jumps.
- **Speaker Notes**: "Our optimizer implements six distinct passes, running iteratively until no further transformations can be safely applied."

---

### **Slide 11: Demonstration: Before vs After Optimization**
- **Input Source Code**:
  ```c
  int x = 3 + 4;
  int y = x + 0;
  while (y < 20) {
      y = y + 1;
  }
  ```
- **Raw TAC (11 Instructions)** vs **Optimized TAC (8 Instructions)**:
  - `t1 = 3 + 4; x = t1` $\to$ `x = 7`
  - `t2 = x + 0; y = t2` $\to$ `y = 7`
  - `t4 = y + 1; y = t4` $\to$ `y = y + 1`
- **Result**: 27.27% instruction reduction with identical semantics!
- **Speaker Notes**: "Notice how temporaries are eliminated, constants are folded, and algebraic identities simplify the instructions while leaving the while loop intact."

---

### **Slide 12: Comprehensive Testing**
- 100% automated test suite (`tests/test_runner.c`).
- 54 discrete assertions covering:
  - Lexer tests (keywords, numbers, comments, invalid characters).
  - Parser & AST tests (precedence, parentheses, if-else, while).
  - Symbol table tests (insertion, shadowing, duplicates).
  - Semantic tests (undeclared variables, type mismatches).
  - TAC generation tests (instruction sequencing).
  - Optimization tests (all 6 rules verified).
  - Boundary tests (empty files, nested parentheses, nested blocks).
- **Speaker Notes**: "Every phase is validated through 54 programmatic unit assertions, ensuring no feature is faked and regressions are caught."

---

### **Slide 13: Results & Empirical Evaluation**
- **Test Suite Result**: 54 / 54 tests passed (100% success rate, 0 failures).
- **Compiler Warnings**: Zero warnings with `-Wall -Wextra -Wpedantic`.
- **Measured Reductions**:
  - `examples/basic.mc`: 27.27% reduction.
  - `examples/optimization.mc`: 44.44% reduction (18 $\to$ 10 instructions).
  - `examples/arithmetic.mc`: 15.38% reduction.
- All numbers dynamically generated by the compiler.
- **Speaker Notes**: "Our empirical results demonstrate measurable optimization gains, achieving up to 44.44% reduction on compute-heavy code."

---

### **Slide 14: Limitations and Future Work**
- **Current Limitations**:
  - No arrays or structs.
  - No user-defined functions or recursion.
  - TAC stops prior to native machine code generation.
- **Future Enhancements**:
  - Array indexing and bounds checking.
  - Function frames and calling conventions.
  - Control-Flow Graph (CFG) construction and SSA representation.
  - Back-end code emission for RISC-V or x86-64 assembly.
- **Speaker Notes**: "While intentionally restricted for pedagogical clarity, the modular architecture makes future extensions—such as adding RISC-V code generation—straightforward."

---

### **Slide 15: Conclusion**
- Successfully designed and implemented a genuine, modular, standards-compliant compiler for MiniC in standard C11.
- Demonstrates all foundational Compiler Design concepts from scanning to intermediate code optimization.
- Fully documented, tested, and explainable for academic defense.
- **Thank you! Questions and Discussion.**
- **Speaker Notes**: "Thank you for your time and attention. I am now ready to demonstrate the live compiler and answer your questions."
