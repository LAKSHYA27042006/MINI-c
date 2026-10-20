# Mini-Compiler with Intermediate Code Generation, Peephole Optimization, and Interactive Web Studio (MiniC)
## Final Academic Project Report & Laboratory Manual (Phase 3)

---

### **Table of Contents**
1. [Chapter 1: Introduction](#chapter-1-introduction)
2. [Chapter 2: Problem Statement](#chapter-2-problem-statement)
3. [Chapter 3: Objectives](#chapter-3-objectives)
4. [Chapter 4: Literature Survey / Background Study](#chapter-4-literature-survey--background-study)
5. [Chapter 5: System Design](#chapter-5-system-design)
6. [Chapter 6: Methodology](#chapter-6-methodology)
7. [Chapter 7: Compiler Design Concepts Used](#chapter-7-compiler-design-concepts-used)
8. [Chapter 8: Implementation Details](#chapter-8-implementation-details)
9. [Chapter 9: Testing and Results](#chapter-9-testing-and-results)
10. [Chapter 10: Conclusion](#chapter-10-conclusion)
11. [Chapter 11: Future Enhancements](#chapter-11-future-enhancements)
12. [References](#references)
13. [Appendix](#appendix)

---

## Chapter 1: Introduction

### 1.1 Overview of Compiler Design
A compiler is a foundational software system that translates a program written in a high-level programming language (source language) into an equivalent program in a low-level target language (such as intermediate representation, bytecode, or machine code). The compilation process is structured into multiple sequential and interdependent phases, logically divided into the **Front-End**, **Middle-End**, and **Back-End**:

1. **Front-End (Language-Dependent)**:
   - **Lexical Analysis (Scanning)**: Converts a raw stream of characters into recognized tokens, filtering out comments and whitespace, while tracking source coordinates (line and column numbers).
   - **Syntax Analysis (Parsing)**: Validates token sequences against a formal context-free grammar and produces an Abstract Syntax Tree (AST).
   - **Semantic Analysis (Type and Scope Checking)**: Validates static semantics, checks type consistency, resolves variable declarations, and constructs scoped symbol tables.

2. **Middle-End (Machine-Independent)**:
   - **Intermediate Code Generation (ICG)**: Translates the decorated AST into an Intermediate Representation (IR), notably Three-Address Code (TAC), which decouples target architecture specifics from source language semantics.
   - **Machine-Independent Code Optimization**: Applies program transformations (e.g., constant folding, constant propagation, algebraic simplification, redundant temporary elimination, dead code elimination, and unreachable code elimination) to improve execution efficiency, reduce instruction counts, and minimize resource overhead without altering program observable behavior.

3. **Back-End (Machine-Dependent)**:
   - Target code generation, register allocation, instruction scheduling, and machine code emission.

### 1.2 Motivation for the MiniC Project
Modern industrial compilers like GCC, Clang/LLVM, and MSVC consist of millions of lines of code. While industrially potent, their sheer complexity obscures the core mathematical and algorithmic concepts of compiler design for students and researchers.

The **MiniC Compiler** project is designed from first principles in standard **C11** without relying on third-party frameworks or compiler-compiler generators (such as Lex/Yacc or Flex/Bison). By focusing on a well-defined, restricted C-like subset (MiniC) and intentionally stopping at optimized Three-Address Code (TAC), this project isolates and illuminates the mechanics of lexical scanning, recursive descent parsing, AST construction, scoped symbol tables, TAC generation, and multi-pass peephole optimizations.

In Phase 3, the project completes the integration by introducing **MiniC Compiler Studio**: a full-stack, responsive web frontend and local API bridge that enables visual step-by-step pipeline inspection, side-by-side TAC diffing, and real-time optimization telemetry in the browser.

---

## Chapter 2: Problem Statement

### 2.1 The Challenge of Industrial Compiler Architectures
Full C compilers (supporting ISO C99/C11/C17) require intricate implementations of preprocessor macros, complicated pointer arithmetic, memory alignments, dynamic memory allocation models, union layouts, calling conventions, ABI compatibility, and architecture-specific machine code emission. Developing such a compiler from scratch within an academic timeframe leads to fragmented implementations where core conceptual learning is compromised.

### 2.2 The MiniC Solution
The MiniC project addresses this problem by designing a rigorous, manageable, and academically transparent compiler that:
- Restricts language features to a focused subset: primitive types (`int`, `float`), explicit variable declarations and initializations, standard arithmetic and relational operators with operator precedence, `if-else` conditional branching, `while` iterative loops, and lexical block scoping.
- Omits unnecessary complexities such as pointers, structures, unions, preprocessor macros, and machine-dependent code generation.
- Concludes at the Intermediate Representation (Three-Address Code) and applies structured peephole optimization passes.
- Provides dynamic metrics demonstrating intermediate code size reduction and transformation frequency.
- Integrates a visual Web Studio frontend to demonstrate the internal workings to academic evaluators.

---

## Chapter 3: Objectives

The concrete, measurable objectives of the MiniC Compiler project across all phases are:

1. **Manual Lexical Analyzer**: Implement a deterministic scanner in C11 that tokenizes source code, handles whitespace and nested comments, tracks line and column numbers, and detects lexical errors with precise diagnostics.
2. **Recursive Descent Parser**: Construct an LL(1) / predictive recursive descent parser based on a formal context-free grammar that enforces operator precedence and parenthetical grouping.
3. **Abstract Syntax Tree (AST)**: Formulate an explicit, dynamic AST node hierarchy in C to represent statements, expressions, control flow, and literals.
4. **Scoped Symbol Table Management**: Implement a scope-stack symbol table supporting nested block scopes, variable insertion, resolution, scope shadowing, and duplicate declaration detection.
5. **Static Semantic Analysis**: Validate that identifiers are declared before use, enforce type compatibility across assignments and operations (e.g., verifying integer-only modulo `%`), and detect semantic anomalies.
6. **Structured Three-Address Code Generation**: Translate AST constructs into structured TAC instructions featuring temporaries (`t1`, `t2`, ...), labels (`L1`, `L2`, ...), and explicit control-flow branches.
7. **Peephole Optimization Engine**: Implement a multi-pass sliding-window optimizer executing:
   - Constant Folding
   - Algebraic Simplification
   - Constant Propagation
   - Redundant Temporary Elimination
   - Dead Code Elimination
   - Unreachable Code Removal
8. **Dynamic Optimization Metrics**: Dynamically record and display instruction reductions, pass counts, and percentage efficiency gains.
9. **Interactive Web Studio Frontend (Phase 3 Innovation)**: Build a responsive, browser-based Compiler Studio with an interactive code editor, 6-stage pipeline stepper, token badges, AST visualizer, symbol table directory, side-by-side TAC diff viewer, and pass breakdown charts.
10. **Academic Verification & Robustness**: Build a 100% automated test suite verifying edge cases, syntax errors, semantic failures, and optimization correctness.

---

## Chapter 4: Literature Survey / Background Study

### 4.1 Classical Compiler Theory
The conceptual framework of this compiler derives from established compiler theory:
- **Aho, Lam, Sethi, and Ullman (2006)** (*Compilers: Principles, Techniques, and Tools*, 2nd Edition): Establishes the classic front-to-back pipeline, LL parsing models, syntax-directed translation, and intermediate representations.
- **Cooper and Torczon (2011)** (*Engineering a Compiler*, 2nd Edition): Highlights the central role of intermediate representations (IR) in decoupling source analysis from target code generation and details local optimization algorithms.
- **Muchnick (1997)** (*Advanced Compiler Design and Implementation*): Outlines peephole optimization strategies, control-flow graph (CFG) simplifications, and constant propagation mechanisms.

### 4.2 Intermediate Representations & Three-Address Code
Three-Address Code (TAC) is a linearized representation of an Abstract Syntax Tree where each instruction has at most one operator and at most three operands (two sources and one destination). TAC resembles assembly code but retains symbolic variables and unbounded virtual registers (temporaries). This makes TAC the premier representation for machine-independent code optimization.

### 4.3 Peephole Optimization Techniques
Peephole optimization is a local optimization technique introduced by W. M. McKeeman (1965) that inspects a short sequence of target or intermediate instructions (the "peephole" window) and replaces them with a shorter or faster equivalent sequence. The MiniC optimizer implements this concept across sliding windows and iterative basic-block scans.

---

## Chapter 5: System Design

### 5.1 System Architecture
The MiniC compiler operates as a strictly staged pipeline where each phase consumes the data structures produced by the preceding phase, with an integrated Web Studio frontend connecting via a local API bridge:

```mermaid
flowchart TD
    WebUI["MiniC Web Studio (HTML5/CSS3/JS)"] <--> Server["Local Python Server (server.py)"]
    Server <--> CLI["MiniC Core Executable (minic.exe)"]
    
    subgraph Compiler Core Pipeline (C11)
        Src["MiniC Source (.mc)"] --> Lexer["Lexical Analyzer (lexer.c)"]
        Lexer --> Tokens["Token Stream (token.c)"]
        Tokens --> Parser["Recursive Descent Parser (parser.c)"]
        Parser --> AST["Abstract Syntax Tree (ast.c)"]
        AST --> SemCheck["Semantic Analyzer (semantic.c)"]
        SemCheck <--> SymTab["Scoped Symbol Table (symbol_table.c)"]
        SemCheck --> TACGen["TAC Generator (tac.c)"]
        TACGen --> RawTAC["Raw Three-Address Code"]
        RawTAC --> Optimizer["Peephole Optimizer (optimizer.c)"]
        Optimizer --> OptTAC["Optimized Three-Address Code"]
        Optimizer --> Report["Dynamic Telemetry Report"]
    end
```

### 5.2 Module Deconstruction
- **`error`**: Handles error records, classifies errors (Lexical, Syntax, Semantic, Warning), and tracks diagnostic locations.
- **`token`**: Defines token types, token structures, literal unions, and dynamic token array manipulation.
- **`lexer`**: Character-level stream processing, comment removal, keyword matching, and coordinate accounting.
- **`ast`**: Memory representations of grammar nodes, child-parent linkages, AST pretty printing, and tree destruction.
- **`parser`**: Recursive descent grammar implementation enforcing precedence and emitting AST nodes.
- **`symbol_table`**: Scoped hash/list records, scope-level stacks, parent links, and symbol resolution.
- **`semantic`**: AST traversal verifying symbol validity, operand types, and block boundaries.
- **`tac`**: Structured instructions (`TAC_ADD`, `TAC_ASSIGN`, `TAC_IF_FALSE`, `TAC_LABEL`, etc.), operand unions, temporary variable generators, and label allocators.
- **`optimizer`**: Multi-pass peephole optimizer executing algebraic, constant, temporary, and dead-code reduction algorithms.
- **`main`**: CLI flag parsing, file reading, interactive prompt loop, and compilation coordinator.
- **`server.py`**: Local HTTP server and API bridge communicating with `minic.exe` and serving the frontend.
- **`web/`**: Interactive Compiler Studio UI (`index.html`, `styles.css`, `app.js`).

---

## Chapter 6: Methodology

1. **Source Ingestion**: The complete source file is read into a contiguous heap buffer to allow forward scanning and character peeking without disk I/O bottlenecks.
2. **Lexical Scanning**: The scanner sequentially examines characters. Whitespace and comments (`//` and `/* ... */`) are stripped. Tokens are generated for keywords, identifiers, numeric literals, operators, and delimiters.
3. **Syntax Analysis**: The parser implements grammar rules using mutually recursive functions. Precedence climbing ensures that multiplication, division, and modulo bind tighter than addition and subtraction. Parentheses dynamically override default precedence.
4. **AST Generation**: Rather than executing code during parsing, the parser constructs a clean AST where parent nodes represent operations and leaf nodes represent literals or identifier references.
5. **Semantic Validation**: A tree walker traverses the AST, inserting symbols into the current scope level of the Symbol Table, checking identifier validity, catching duplicate declarations, and verifying type consistency.
6. **TAC Linearization**: A syntax-directed code generator walks the validated AST and emits linearized Three-Address Code with explicit temporaries (`t1`, `t2`, ...) and control-flow labels (`L1`, `L2`, ...).
7. **Peephole Optimization**: The optimizer iterates over the TAC linked list, applying local transformation patterns until a fixed point is reached.
8. **Interactive Visual Reporting**: The compiled output is formatted for both terminal CLI output and the Web Studio frontend.

---

## Chapter 7: Compiler Design Concepts Used

### 7.1 Lexical Analysis & Finite State Concept
- **Definition**: The transformation of an input character stream into a sequence of atomic semantic units called tokens.
- **Implementation**: Implemented as a stateful scanner tracking character coordinates (`line`, `column`).
- **Example**: `int x = 3 + 4;` yields:
  `KEYWORD_INT`, `IDENTIFIER("x")`, `ASSIGN`, `INT_LITERAL(3)`, `PLUS`, `INT_LITERAL(4)`, `SEMICOLON`, `EOF`.

### 7.2 Context-Free Grammars & Recursive Descent Parsing
- **Definition**: A top-down parsing technique where grammar productions are represented as recursive C functions.
- **Grammar Enforced**:
  - $E \to T \; ((+ \mid -) \; T)^*$
  - $T \to F \; ((* \mid / \mid \%) \; F)^*$
  - $F \to \text{id} \mid \text{num} \mid (E) \mid (+ \mid -) F$
- **Example**: `a + b * c` is parsed with `b * c` as a subtree under `+`.

### 7.3 Scoped Symbol Table Management
- **Definition**: A data structure that stores variable attributes (name, data type, declaration location, scope depth, initialization status).
- **Scope Model**: Implements a scope stack with parent pointers. Entering a `{` pushes a new scope level; exiting a `}` pops to the parent scope, properly implementing lexical scoping and shadowing.

### 7.4 Three-Address Code (TAC) Intermediate Representation
- **Definition**: A linearized, language-independent IR where instructions have at most three address operands.
- **Structure**:
  - Operation: `TACOp` (`TAC_ADD`, `TAC_ASSIGN`, `TAC_IF_FALSE`, etc.)
  - Result: `TACOperand`
  - Argument 1: `TACOperand`
  - Argument 2: `TACOperand`
- **Example**: `while (y < 20) { y = y + 1; }` translates to:
  ```
  L1:
      t3 = y < 20
      ifFalse t3 goto L2
      t4 = y + 1
      y = t4
      goto L1
  L2:
  ```

### 7.5 Peephole Optimization Passes
1. **Constant Folding**: Evaluates constant expressions at compile time.
   - Example: `t1 = 3 + 4` $\to$ `t1 = 7`.
2. **Algebraic Simplification**: Uses identity and annihilator algebraic properties:
   - $x + 0 \to x$, $x - 0 \to x$, $x \times 1 \to x$, $x / 1 \to x$
   - $x \times 0 \to 0$, $x - x \to 0$
3. **Constant Propagation**: Propagates known constant values to downstream uses within the basic block.
   - Example: `x = 10; y = x + 5` $\to$ `y = 10 + 5` $\to$ `y = 15`.
4. **Redundant Temporary Elimination**: Eliminates unnecessary temporary variables:
   - `t1 = a + b; x = t1` $\to$ `x = a + b`.
5. **Dead Code Elimination**: Removes instructions whose results are never read or overwritten prior to use.
   - Example: `x = 100; x = 200;` $\to$ `x = 200;`.
6. **Unreachable Code Elimination**: Removes instructions immediately following an unconditional jump (`goto`) up to the next target label.

---

## Chapter 8: Implementation Details

### 8.1 Toolchain and Standard
- **Core Compiler**: Standard C (C11, ISO/IEC 9899:2011).
- **C Compiler**: MinGW GCC 6.3.0 / GNU GCC (`-std=c11 -Wall -Wextra -Wpedantic -O2 -Isrc`).
- **Frontend Stack**: HTML5, CSS3 (responsive dark theme), Vanilla JavaScript (ES6+).
- **Local Server**: Python 3.13 Standard Library (`http.server`, `subprocess`).

### 8.2 Project Structure
```
MiniC/
├── Makefile                          # Portable build configuration (make / mingw32-make)
├── README.md                         # Project documentation and CLI usage guide
├── .gitignore                        # Ignore rules for build objects and binaries
├── server.py                         # Local Python API server and compiler bridge
│
├── src/                              # Core Compiler Engine (C11)
│   ├── main.c                        # CLI flags, argument parsing, interactive menu
│   ├── error.h / error.c             # Error collection and diagnostic reporting
│   ├── token.h / token.c             # Token structures and dynamic arrays
│   ├── lexer.h / lexer.c             # Lexical scanner and coordinate tracker
│   ├── ast.h / ast.c                 # Abstract Syntax Tree nodes and printer
│   ├── parser.h / parser.c           # Predictive recursive descent parser
│   ├── symbol_table.h / symbol_table.c # Scoped symbol table and scope stack
│   ├── semantic.h / semantic.c       # Semantic analyzer and type checker
│   ├── tac.h / tac.c                 # Three-Address Code generator
│   └── optimizer.h / optimizer.c     # Multi-pass peephole optimizer
│
├── web/                              # Interactive Web Studio Frontend
│   ├── index.html                    # Single-page web studio interface
│   ├── styles.css                    # Dark IDE theme styles
│   └── app.js                        # Client-side controller and API handler
│
├── examples/                         # Verified Sample Programs
│   ├── basic.mc                      # Primary demonstration program
│   ├── arithmetic.mc                 # Operator precedence test cases
│   ├── control_flow.mc               # if-else and while loop test cases
│   ├── optimization.mc               # Peephole optimization showcase
│   ├── nested_expression.mc          # Mixed-type float expressions
│   └── errors.mc                     # Diagnostic error test cases
│
├── tests/                            # Automated Regression Test Suite
│   ├── test_runner.c                 # Automated test harness (54 assertions)
│   └── *.mc                          # Individual test programs
│
└── docs/                             # Academic Documentation Deliverables
    ├── DOCUMENTATION.md              # 11-Chapter Academic Report
    ├── VIVA_QUESTIONS.md             # 45 Viva Voce Questions & Answers
    └── PRESENTATION_CONTENT.md       # 15-Slide Presentation Deck
```

---

## Chapter 9: Testing and Results

### 9.1 Test Matrix & Automated Verification

The automated test runner (`tests/test_runner.c`) executes 54 assertions covering every compiler phase with 100% pass rate:

| Test Suite | Total Assertions | Passed | Failed | Success Rate |
| :--- | :---: | :---: | :---: | :---: |
| **Lexer Suite** | 10 | 10 | 0 | 100% |
| **Parser & AST Suite** | 10 | 10 | 0 | 100% |
| **Symbol Table Suite** | 6 | 6 | 0 | 100% |
| **Semantic Analysis Suite** | 8 | 8 | 0 | 100% |
| **TAC Generation Suite** | 4 | 4 | 0 | 100% |
| **Peephole Optimization Suite** | 8 | 8 | 0 | 100% |
| **Error Handling & Boundary Suite** | 8 | 8 | 0 | 100% |
| **Total** | **54** | **54** | **0** | **100%** |

### 9.2 Functional Test Cases

| Case ID | Input Category | Test Description | Expected Behavior | Actual Behavior | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-01** | Lexical | Scans identifiers and keywords (`int`, `float`, `while`) | Distinct token types emitted | Correct token stream generated | **PASS** |
| **TC-02** | Lexical | Handles single-line (`//`) and multi-line (`/* */`) comments | Comments skipped without tokens | Comments cleanly stripped | **PASS** |
| **TC-03** | Lexical | Unrecognized character `@` | Lexical error reported with line/col | Error reported: Unexpected character '@' | **PASS** |
| **TC-04** | Syntax | Precedence `2 + 3 * 4` | Root node `+`, right child `*` | Verified AST hierarchy | **PASS** |
| **TC-05** | Syntax | Parentheses `(2 + 3) * 4` | Root node `*`, left child `+` | Verified AST hierarchy | **PASS** |
| **TC-06** | Syntax | Missing semicolon `int x = 10` | Syntax error diagnostic | Syntax error: Expected ';' | **PASS** |
| **TC-07** | Semantic | Undeclared variable `x = 5;` | Semantic error diagnostic | Semantic error: Variable 'x' not declared | **PASS** |
| **TC-08** | Semantic | Duplicate declaration `int x; int x;` | Duplicate declaration error | Semantic error: Already declared in current scope | **PASS** |
| **TC-09** | Semantic | Modulo on float `float f; int r = f % 2;` | Modulo type mismatch error | Semantic error: Binary '%' requires integer operands | **PASS** |
| **TC-10** | Semantic | Block scoping & variable shadowing | Inner declaration shadows outer | Inner scope resolved to float, restored on exit | **PASS** |
| **TC-11** | TAC | While loop translation | Conditional jump, body, backward jump | Correct `ifFalse`, `goto`, and labels emitted | **PASS** |
| **TC-12** | Optimization | Constant folding `int x = 3 + 4;` | Transformed to `x = 7` | Folded to 7, temporary eliminated | **PASS** |
| **TC-13** | Optimization | Algebraic simplification `a + 0`, `a * 1` | Replaced with `a` | Simplified to direct assignments | **PASS** |
| **TC-14** | Optimization | Constant propagation `x = 10; y = x + 5;` | Propagates 10 to fold `y = 15` | `y` directly assigned 15 | **PASS** |
| **TC-15** | Optimization | Dead code elimination `x = 10; x = 20;` | Initial store `x = 10` removed | Dead assignment eliminated | **PASS** |
| **TC-16** | Optimization | Division by zero `int x = 10 / 0;` | Safely NOT folded | Preserved without crash | **PASS** |
| **TC-17** | Boundary | Empty source code | Valid compilation, 0 statements | Handled cleanly | **PASS** |
| **TC-18** | Boundary | Deeply nested blocks (4 levels) | Correct scope entry and exit | Handled cleanly | **PASS** |

### 9.3 Performance Evaluation & Optimization Telemetry

| Program File | Original TAC Instructions | Optimized TAC Instructions | Instructions Removed | Reduction Percentage | Execution Time (ms) |
| :--- | :---: | :---: | :---: | :---: | :---: |
| `examples/basic.mc` | 11 | 8 | 3 | **27.27%** | 2.4 ms |
| `examples/optimization.mc` | 18 | 10 | 8 | **44.44%** | 3.1 ms |
| `examples/arithmetic.mc` | 13 | 11 | 2 | **15.38%** | 2.1 ms |
| `examples/control_flow.mc` | 12 | 10 | 2 | **16.67%** | 2.8 ms |

---

## Chapter 10: Conclusion

The MiniC Compiler project successfully achieves all planned academic and architectural objectives across all three project phases. Developed from first principles in standard C11:
1. It implements a complete, transparent, and modular front-to-middle compiler pipeline.
2. It enforces lexical, syntactic, and semantic rules with informative error messages.
3. It translates high-level structured control flow and arithmetic expressions into clean Three-Address Code.
4. It features a multi-pass peephole optimizer executing constant folding, algebraic simplification, constant propagation, redundant temporary elimination, dead code elimination, and unreachable code elimination.
5. All optimization statistics are calculated dynamically from actual instruction transformations.
6. Phase 3 introduces the **MiniC Compiler Studio**, enabling visual, interactive exploration of every compiler stage directly in the browser.
7. The project is 100% verified with automated tests, compiles with zero warnings under `-Wall -Wextra -Wpedantic`, and adheres strictly to software engineering best practices.

---

## Chapter 11: Future Enhancements

The following extensions are documented as valuable directions for future development:
1. **Array Support**: Adding fixed-size one-dimensional and multi-dimensional array declarations (`int arr[10];`) with index bounds checking in TAC.
2. **Function Definitions & Calls**: Implementing procedure declarations, formal parameters, call frames, return values, and recursion.
3. **Control-Flow Graph (CFG) & SSA Representation**: Constructing explicit basic-block graphs and translating TAC into Static Single Assignment (SSA) form with $\phi$-functions.
4. **Data-Flow Analysis Framework**: Global data-flow analysis (available expressions, reaching definitions, live variable analysis) beyond straight-line basic blocks.
5. **Code Generation to Real Machine Targets**: Adding a back-end emitter translating optimized TAC into target assembly (such as RISC-V, MIPS, or x86-64).

---

## References

1. **Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D.** (2006). *Compilers: Principles, Techniques, and Tools* (2nd ed.). Boston, MA: Addison-Wesley. (The Dragon Book).
2. **Cooper, K. D., & Torczon, L.** (2011). *Engineering a Compiler* (2nd ed.). Burlington, MA: Morgan Kaufmann.
3. **Muchnick, S. S.** (1997). *Advanced Compiler Design and Implementation*. San Francisco, CA: Morgan Kaufmann.
4. **McKeeman, W. M.** (1965). *Peephole Optimization*. Communications of the ACM, 8(7), 443–444.
5. **ISO/IEC 9899:2011**. (2011). *Information technology — Programming languages — C*. International Organization for Standardization.

---

## Appendix

### Appendix A: Formal Grammar of MiniC (EBNF)
```ebnf
program             ::= statement* EOF ;

statement           ::= var_declaration
                      | assignment
                      | if_statement
                      | while_statement
                      | block ;

var_declaration     ::= type IDENTIFIER ( '=' expression )? ';' ;

type                ::= 'int' | 'float' ;

assignment          ::= IDENTIFIER '=' expression ';' ;

if_statement        ::= 'if' '(' condition ')' statement ( 'else' statement )? ;

while_statement     ::= 'while' '(' condition ')' statement ;

block               ::= '{' statement* '}' ;

condition           ::= expression ( ( '<' | '>' | '<=' | '>=' | '==' | '!=' ) expression )? ;

expression          ::= term ( ( '+' | '-' ) term )* ;

term                ::= factor ( ( '*' | '/' | '%' ) factor )* ;

factor              ::= INTEGER_LITERAL
                      | FLOAT_LITERAL
                      | IDENTIFIER
                      | '(' expression ')'
                      | ( '+' | '-' ) factor ;
```
