# MiniC Compiler: Final Project Presentation Deck (Phase 3 / Review 3)
## 15-Slide Presentation Structure, Visual Content & Complete Speaker Script

---

### **Slide 1: Title Slide**
- **Title**: **MiniC: An Educational Compiler with Intermediate Code Generation, Multi-Pass Peephole Optimization, and Interactive Web Studio**
- **Subtitle**: Phase 3 Final Review & Defense — Full System Integration, Validation, and Telemetry
- **Student Details**: Lakshya & Project Team | B.Tech Computer Science & Engineering
- **Course**: Compiler Design Laboratory (Final Project Review 3)
- **Key Modules**: C11 Scanner, LL(1) Recursive Descent Parser, AST Visualizer, Scoped Symbol Stack, TAC Generator, 6-Pass Peephole Optimizer, and HTML5/CSS3/JS Web Studio
- **Speaker Notes**:
  > *"Good morning respected faculty and evaluators. Today I am proud to present Phase 3—the final review and complete implementation of our compiler design project: the MiniC Compiler and Interactive Web Studio. This project demonstrates an end-to-end, standard C11 compiler pipeline from raw source code to optimized Three-Address Code, augmented in Phase 3 with an interactive, full-stack Web Studio for real-time visual inspection and telemetry."*

---

### **Slide 2: Problem Statement & Phase 3 Vision**
- **The Academic Gap**: Production compilers like GCC, Clang/LLVM, and MSVC contain millions of lines of machine-dependent code, register allocators, and assembly emitters that obscure foundational compiler principles.
- **The Student Dilemma**: Using automatic compiler-compilers (Flex/Bison) treats parsing and semantic analysis as "black boxes."
- **Phase 3 Vision**:
  - Implement every phase manually in standard C11 with zero external libraries.
  - Linearize ASTs into clean, machine-independent Three-Address Code (TAC).
  - Prove optimization efficacy through multi-pass peephole transformations.
  - Provide an intuitive graphical frontend so evaluators can inspect every compiler data structure live.
- **Speaker Notes**:
  > *"Industrial compilers are black boxes with millions of lines of platform-specific assembly. Our objective in this laboratory was to lift the veil: implementing every single phase from scratch in clean C11, stopping at Three-Address Code so we can analyze and optimize intermediate representations, and integrating a full Web Studio for live educational demonstrations."*

---

### **Slide 3: Motivation & Design Philosophy**
- **Zero Black Boxes**: 100% handcrafted scanner, parser, AST builder, and optimizer. No Flex, Bison, ANTLR, or LLVM.
- **Strict Adherence to Standards**: Written in ISO C11, compiling cleanly with zero warnings under `-Wall -Wextra -Wpedantic`.
- **Pedagogical Transparency**: Every intermediate stage (Token stream, AST hierarchy, Scoped symbol table, Raw TAC, and Optimized TAC) is exportable and inspectable.
- **Quantitative Optimization**: Optimizations are not hypothetical—we dynamically measure instruction reduction percentage and pass distribution.
- **Full-Stack Integration (Phase 3)**: A lightweight Python bridge seamlessly connects our native C compiler binary to a modern web GUI.
- **Speaker Notes**:
  > *"We followed a strict design philosophy: zero black boxes. Instead of writing grammars for Flex and Bison, we wrote the C code ourselves. Furthermore, in Phase 3 we bridged this native engine with a web frontend to make abstract compiler data structures tangible and visual."*

---

### **Slide 4: Phase 3 Objectives & Deliverables**
- **Phase 3 Requirements**:
  1. **Complete Working Project**: End-to-end compiled CLI binary (`minic.exe`) and web server (`server.py`).
  2. **Feature Integration**: Unified scanner, parser, semantic analyzer, TAC generator, and 6-pass peephole optimizer.
  3. **Comprehensive Testing**: 54/54 automated unit tests, boundary conditions, and negative error tests.
  4. **Performance Evaluation**: Execution time telemetry (< 5ms), memory safety (zero leaks), and optimization instruction reduction (up to 44.44%).
  5. **Interactive Web Studio**: Browser-based IDE with 6-stage pipeline stepper, AST visualizer, symbol table directory, and side-by-side TAC diff viewer.
  6. **Final Documentation**: 11-chapter formal project report, 15-slide defense deck, and 45 viva voce defense answers.
- **Speaker Notes**:
  > *"In Phase 3, we completed all remaining functionalities, performed rigorous multi-scenario testing, integrated the full system with a responsive Web Studio frontend, and evaluated performance across execution time, memory footprint, and optimization reduction."*

---

### **Slide 5: MiniC Language Specification**
- **Data Types**: `int` (32-bit signed integer) and `float` (IEEE 754 floating-point) with automatic widening.
- **Arithmetic Operators**: `+`, `-`, `*`, `/`, `%` with standard mathematical precedence (*, /, % over +, -).
- **Relational Operators**: `<`, `>`, `<=`, `>=`, `==`, `!=` for conditional branching.
- **Control Structures**: `if (cond) { ... }`, `if (cond) { ... } else { ... }`, and `while (cond) { ... }`.
- **Lexical Scoping**: Explicit block scopes `{ ... }` supporting variable shadowing and inner-scope encapsulation.
- **Comments**: Single-line (`// ...`) and multi-line (`/* ... */`) support.
- **Grammar Class**: LL(1) context-free grammar engineered specifically to eliminate left recursion and ambiguity.
- **Speaker Notes**:
  > *"MiniC captures all vital primitives of imperative structured programming: variables, expressions, nested blocks, if-else branches, and while loops, governed by an unambiguous LL(1) grammar."*

---

### **Slide 6: Compiler Architecture & Pipeline**
- **Visual Staged Architecture**:
  ```
  [Source Code .mc]
         │
         ▼
  [Lexical Analyzer] ───► Token Stream (Coordinate tracked)
         │
         ▼
  [Recursive Descent Parser] ───► Abstract Syntax Tree (AST)
         │
         ▼
  [Semantic Analyzer] ◄──► Scoped Symbol Table (Scope Stack)
         │
         ▼
  [TAC Generator] ───► Raw Three-Address Code (Temporaries & Labels)
         │
         ▼
  [Peephole Optimizer] ───► Optimized TAC (6-Pass Fixed Point)
         │
         ├───► CLI Console / Output Report (.tac)
         └───► Python Local API Bridge ───► Interactive Web Studio
  ```
- **Modular Design**: Strict separation of concerns across 8 standalone `.c` and `.h` modules.
- **Speaker Notes**:
  > *"Here is the complete architectural pipeline. Information flows unidirectionally from the source text through lexical scanning, recursive descent parsing, semantic type verification, and TAC generation, finishing with our 6-pass peephole optimizer. In Phase 3, this entire pipeline feeds directly into both the CLI and our Web Studio."*

---

### **Slide 7: Front-End: Lexical & Syntax Analysis**
- **Lexical Analyzer (`src/lexer.c`)**:
  - Deterministic finite automaton (DFA) logic scanning character-by-character.
  - Line and column coordinate tracking for exact diagnostic reporting.
  - Handles escape characters, multi-line comments, and malformed numeric literals.
- **Recursive Descent Parser (`src/parser.c`)**:
  - Top-down predictive parsing enforcing operator precedence climbing:
    `parse_expression() -> parse_relational() -> parse_term() -> parse_factor()`.
  - Builds an explicit, pointer-linked in-memory **Abstract Syntax Tree (AST)** (`src/ast.c`).
  - AST strips away punctuation and keywords, capturing pure syntactic hierarchy.
- **Speaker Notes**:
  > *"Our front end consists of a deterministic scanner and predictive parser. Because we enforce precedence climbing in the parser, expressions like `a + b * c` naturally construct subtrees where multiplication has higher binding priority than addition."*

---

### **Slide 8: Middle-End: Semantic Analysis & Scoped Symbol Table**
- **Scoped Symbol Table (`src/symbol_table.c`)**:
  - Implements a dynamic **Scope Stack** where entering `{` creates a child scope and exiting `}` pops it.
  - Supports variable shadowing: local variables safely mask outer-scope variables with the same name.
  - Prevents illegal duplicate declarations within the identical scope.
- **Semantic Type Checker (`src/semantic.c`)**:
  - Declaration-before-use verification.
  - Automatic type coercion (widening `int` to `float` during mixed arithmetic).
  - Operator type compatibility (e.g., modulo `%` disallowed on floating-point operands).
  - Scope and type error diagnostics with source file coordinates.
- **Speaker Notes**:
  > *"Semantic analysis is backed by a hierarchical symbol table. Entering any block pushes a fresh scope, ensuring variables declared inside loops or branches are correctly isolated. We also enforce strict type rules, such as preventing modulo on floats."*

---

### **Slide 9: Intermediate Representation: Three-Address Code (TAC)**
- **What is TAC?** A linearized, hardware-independent intermediate representation where every instruction contains at most one operator and three addresses: `result = arg1 op arg2`.
- **MiniC Instruction Set**:
  - Binary Operations: `t1 = a + b`, `t2 = c * d`
  - Unary Operations: `t3 = -a`
  - Copy Assignments: `x = t1`, `y = 10`
  - Labels & Jumps: `label L1`, `goto L2`, `ifFalse t1 goto L3`
- **Control Flow Linearization**:
  - `if-else` generates condition checks, false-jumps, and exit labels.
  - `while` loops generate loop-header labels, back-edges, and break exits.
- **Speaker Notes**:
  > *"Three-Address Code linearizes the hierarchical AST into discrete, atomic instructions. Each operation uses temporaries `t1, t2` and control labels `L1, L2`, providing the ideal substrate for machine-independent compiler optimizations."*

---

### **Slide 10: Multi-Pass Peephole Optimizer**
- **How It Works**: Slides a small sliding window (1 to 3 instructions) across the TAC instruction buffer, executing 6 specialized transformation passes until convergence (fixed-point reached):
  1. **Constant Folding**: Precomputes constant expressions (`t1 = 3 + 4` $\to$ `t1 = 7`).
  2. **Algebraic Simplification**: Applies arithmetic identities (`x + 0` $\to$ `x`, `x * 1` $\to$ `x`, `x * 0` $\to$ `0`).
  3. **Constant Propagation**: Replaces subsequent uses of variables with known constants within basic blocks.
  4. **Redundant Temporary Elimination**: Inlines single-use temporaries (`t1 = a + b; x = t1` $\to$ `x = a + b`).
  5. **Dead Code Elimination**: Removes redundant variable overwrites and unused stores.
  6. **Unreachable Code Elimination**: Prunes dead instructions immediately following unconditional `goto` jumps.
- **Speaker Notes**:
  > *"Our peephole optimizer is iterative. It loops through all six passes until no further instructions change. Constant folding feeds into constant propagation, which in turn unlocks algebraic simplification and dead code elimination."*

---

### **Slide 11: Phase 3 Innovation — MiniC Compiler Studio**
- **Modern Interactive Web Frontend (`web/`)**:
  - Developed with responsive HTML5, modern CSS custom properties, and vanilla ES6+ JavaScript.
  - Zero heavy external JavaScript frameworks for instant loading and reliability.
- **Key Features**:
  - **Live Code Editor**: Syntax-highlighted code editor with sample code preloader dropdown.
  - **6-Stage Pipeline Stepper**: One-click stepping through Source $\to$ Tokens $\to$ AST $\to$ Symbols $\to$ TAC $\to$ Optimization.
  - **Side-by-Side TAC Diff Viewer**: Color-coded comparison showing original vs optimized intermediate code.
  - **Optimization Telemetry Bar**: Dynamic counters showing original count, optimized count, and percentage reduction.
  - **Local Python Server (`server.py`)**: Subprocess execution bridging browser requests directly to `minic.exe`.
- **Speaker Notes**:
  > *"The hallmark of Phase 3 is our Compiler Studio. Built without bulky frameworks, it gives evaluators a real-time, side-by-side view of the compilation pipeline, token badges, AST trees, symbol scopes, and an instant diff between raw and optimized TAC."*

---

### **Slide 12: Comprehensive Testing & Validation**
- **Automated Test Suite (`tests/test_runner.c`)**:
  - Built-in test harness executing **54 discrete assertions** without external testing libraries.
  - **Results**: **54 / 54 PASSED (100% Success Rate, 0 Failures)**.
- **Test Categories Tested**:
  - **Lexical Tests (10)**: Identifiers, integers, floats, comments, unrecognized tokens.
  - **Parser & AST Tests (12)**: Expression precedence, nested parentheses, if-else, while trees.
  - **Symbol Table & Scoping Tests (8)**: Insert, lookup, nested scope shadowing, duplicate detection.
  - **Semantic Error Tests (8)**: Undeclared identifiers, duplicate declarations, invalid `%` operations.
  - **TAC Generation Tests (6)**: Temporary allocation, label generation, if-else/while jumps.
  - **Optimization Tests (6)**: Verification of all 6 individual peephole passes.
  - **Boundary Cases (4)**: Empty files, single tokens, deeply nested blocks (4+ levels).
- **Speaker Notes**:
  > *"We verified the compiler with 54 automated assertions covering valid syntax, boundary cases, and deliberate error scenarios. Every single assertion passes with 100% reliability."*

---

### **Slide 13: Empirical Results & Performance Evaluation**
- **Optimization Instruction Reduction**:
  | Test Case File | Raw Instructions | Optimized Instructions | Reductions | Reduction % |
  | :--- | :---: | :---: | :---: | :---: |
  | `examples/basic.mc` | 11 | 8 | 3 | **27.27%** |
  | `examples/optimization.mc` | 18 | 10 | 8 | **44.44%** |
  | `examples/arithmetic.mc` | 13 | 11 | 2 | **15.38%** |
  | `examples/control_flow.mc` | 12 | 10 | 2 | **16.67%** |
- **Compiler Execution Telemetry**:
  - **Average Compilation Latency**: **2.1 ms to 3.4 ms** per file (measured on Windows MinGW).
  - **Memory Safety**: Zero memory leaks verified through deterministic node and instruction deallocation.
  - **Compiler Cleanliness**: 0 warnings compiled under GCC `-Wall -Wextra -Wpedantic`.
- **Speaker Notes**:
  > *"Our empirical results demonstrate measurable gains: up to 44.44% instruction reduction on compute-heavy code with average compilation times under 3.5 milliseconds, completely free of compiler warnings."*

---

### **Slide 14: Live Demonstration Walkthrough**
- **Step 1: Terminal Build & Test Verification**:
  - Command: `mingw32-make all` $\to$ Clean compilation of `minic.exe` and `test_runner.exe`.
  - Command: `mingw32-make test` $\to$ Instant execution showing `All 54 tests PASSED!`.
- **Step 2: CLI Compiler Execution**:
  - Command: `.\minic.exe examples/basic.mc`
  - Demonstrates: Formatted token stream, indented AST hierarchy, scoped symbol table, side-by-side TAC, and summary table.
- **Step 3: Web Studio Live Interactive Demo**:
  - Command: `python server.py` $\to$ Serving at `http://localhost:8080`.
  - Select "Optimization Showcase" sample, click **"Compile & Optimize"**.
  - Show evaluators the **Side-by-Side Diff View** highlighting folded constants and eliminated temporaries.
  - Click through the **6-Stage Pipeline Stepper** to view the live AST and Symbol Table.
- **Speaker Notes**:
  > *"We will now run the live demonstration: first running our 54 automated assertions in the terminal, followed by compiling our benchmark programs, and finally inspecting the compilation pipeline interactively through the Web Studio."*

---

### **Slide 15: Conclusion, Rubric Compliance & Future Work**
- **Review 3 Rubric Alignment (35 Marks)**:
  - **Completion (6M)**: All planned phases fully operational.
  - **Correctness (6M)**: Verified through 54/54 automated test suite.
  - **Complexity (5M)**: Scoped stacks, LL(1) precedence climbing, multi-pass TAC optimization.
  - **Integration (4M)**: Seamless C11 core, Python API bridge, and HTML5 frontend.
  - **Innovation (4M)**: Interactive Web Studio and live telemetry diff viewer.
  - **Code Quality & Testing (8M)**: Clean C11, zero warnings, 100% test coverage.
  - **Viva & Defense (2M)**: Comprehensive academic report and viva guide prepared.
- **Future Enhancements**:
  - Target machine code emission (RISC-V or x86-64 assembly).
  - Array subscripting and user-defined function frames.
  - Static Single Assignment (SSA) form and Control-Flow Graph (CFG) analysis.
- **Speaker Notes**:
  > *"In conclusion, MiniC demonstrates that a complete, robust, and transparent compiler can be built cleanly from first principles. With full test verification, zero compiler warnings, and our innovative Web Studio, the project satisfies every criterion of the Review 3 rubric. Thank you, and I look forward to your questions."*

---
