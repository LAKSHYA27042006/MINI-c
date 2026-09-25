# Mini-Compiler with Intermediate Code Generation and Peephole Optimization (MiniC)
### Academic Compiler Design Individual Project in Standard C11

[![Language](https://img.shields.io/badge/Language-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)]()
[![Tests](https://img.shields.io/badge/Tests-54%2F54%20Passed-success.svg)]()
[![Standard](https://img.shields.io/badge/Standard-ISO%2FIEC%209899%3A2011-orange.svg)]()

---

## 1. Project Title
**Mini-Compiler with Intermediate Code Generation and Peephole Optimization**  
*Short name:* **MiniC Compiler**

---

## 2. Project Description
The **MiniC Compiler** is an educational, production-quality compiler implemented from first principles in standard **C11** (without Lex/Yacc, Flex/Bison, or third-party libraries). It accepts programs written in **MiniC**—a restricted, strongly-typed imperative subset of C—and processes them through a complete front-to-middle compiler pipeline ending at optimized **Three-Address Code (TAC)** intermediate representation.

The project features a deterministic hand-written scanner, a recursive descent parser enforcing operator precedence, an Abstract Syntax Tree (AST) builder, a scope-stack symbol table supporting block shadowing, a static semantic analyzer, a structured TAC generator, and a multi-pass sliding-window peephole optimizer.

---

## 3. Key Features
- **Pure C11 Implementation**: Built exclusively using standard C libraries (`stdio.h`, `stdlib.h`, `string.h`, `ctype.h`, `stdbool.h`). Zero third-party dependencies.
- **Deterministic Lexical Scanner**: Tracks line and column coordinates, ignores whitespace, strips single-line (`//`) and multi-line (`/* */`) comments, and detects invalid characters.
- **Recursive Descent Parser**: Strict predictive LL(1) grammar parsing with operator precedence climbing (`*`, `/`, `%` over `+`, `-`) and parentheses grouping.
- **Dynamic Abstract Syntax Tree (AST)**: Hierarchical node structures with typed unions and an ASCII tree-printer.
- **Scoped Symbol Table**: Supports global and nested block scopes (`global`, `block-1`, `block-2`), variable shadowing, and duplicate declaration detection.
- **Static Semantic Analysis**: Enforces declaration before use, checks type compatibility, validates conditions, and warns against implicit narrowing.
- **Structured Three-Address Code (TAC)**: Emits linearized TAC instructions using explicit virtual temporaries (`t1`, `t2`, ...) and control-flow labels (`L1`, `L2`, ...).
- **Multi-Pass Peephole Optimizer**:
  1. *Constant Folding* (arithmetic & relational evaluation of constants, division-by-zero protected)
  2. *Algebraic Simplification* ($x+0 \to x$, $x \times 1 \to x$, $x \times 0 \to 0$, $x-0 \to x$, $x/1 \to x$)
  3. *Constant Propagation* (within straight-line basic blocks)
  4. *Redundant Temporary Elimination* ($t_1 = a+b; x = t_1 \to x = a+b$)
  5. *Dead Code Elimination* (overwritten stores and dead temporaries)
  6. *Unreachable Code Elimination* (pruning instructions following unconditional `goto`)
- **Dynamic Optimization Reporting**: Computes instruction reduction counts and percentage savings dynamically from actual code transformations.
- **Automated Test Suite**: 54 programmatic unit assertions across all phases with 100% pass rate.
- **Interactive CLI & Batch Modes**: Supports flag-based compilation (`--tokens`, `--ast`, `--symtab`, `--tac`, `--optimized`, `--report`, `--all`) and an interactive menu.

---

## 4. Compiler Pipeline Architecture

```
                    SOURCE CODE (.mc)
                            │
                            ▼
                  ┌──────────────────┐
                  │ LEXICAL ANALYZER │  (lexer.c)
                  └──────────────────┘
                            │
                            ▼
                       TOKEN STREAM    (token.c)
                            │
                            ▼
                  ┌──────────────────┐
                  │ SYNTAX ANALYZER  │  (parser.c)
                  └──────────────────┘
                            │
                            ▼
                 ABSTRACT SYNTAX TREE   (ast.c)
                            │
                            ▼
                  ┌──────────────────┐
                  │ SEMANTIC CHECKER │  (semantic.c)
                  └──────────────────┘
                            │
                     ┌──────┴──────┐
                     ▼             ▼
              SYMBOL TABLE    VALIDATED AST
             (symbol_table.c)      │
                     │             │
                     └──────┬──────┘
                            │
                            ▼
                  ┌──────────────────┐
                  │  TAC GENERATOR   │  (tac.c)
                  └──────────────────┘
                            │
                            ▼
                 RAW THREE-ADDRESS CODE
                            │
                            ▼
                  ┌──────────────────┐
                  │PEEPHOLE OPTIMIZER│  (optimizer.c)
                  └──────────────────┘
                            │
                            ▼
                 OPTIMIZED THREE-ADDRESS CODE
                            │
                            ▼
                  OPTIMIZATION REPORT
```

---

## 5. Supported MiniC Language Syntax

MiniC supports the following core constructs:

### Data Types
- `int`: Signed integers (e.g., `10`, `-42`).
- `float`: Double-precision floating-point numbers (e.g., `3.14159`, `0.5`).

### Variable Declarations & Initializations
```c
int a;
int x = 10;
float pi = 3.14159;
int total = 3 + 4 * 2;
```

### Assignments & Arithmetic Expressions
```c
x = 20;
y = x + 5;
z = (a + b) * (c - 2) / 4 % 3;
```
*Precedence hierarchy:* `*`, `/`, `%` bind tighter than `+`, `-`. Parentheses `(...)` override precedence.

### Relational Operators & Conditions
Supports `<`, `>`, `<=`, `>=`, `==`, `!=`.
```c
if (x > 10) { ... }
while (count <= 100) { ... }
```

### Control Flow
- `if (condition) statement`
- `if (condition) statement else statement`
- `while (condition) statement`

### Blocks & Scoping
```c
int x = 10;
{
    int x = 20; // Shadows outer 'x' in block scope
    int y = x + 5;
}
// Outer 'x' restored to 10 here
```

### Comments
```c
// Single-line comment
/* Multi-line
   comment */
```

---

## 6. Project Directory Structure

```
MiniC/
├── Makefile                 # Portable build system (make / mingw32-make)
├── README.md                # Project documentation and quickstart
├── .gitignore               # Ignored build binaries and objects
│
├── src/                     # Core compiler implementation in C11
│   ├── main.c               # Entry point, CLI flags, interactive menu
│   ├── error.h / .c         # Diagnostic reporting (Lexical, Syntax, Semantic)
│   ├── token.h / .c         # Token definitions and dynamic token arrays
│   ├── lexer.h / .c         # Deterministic lexical analyzer
│   ├── ast.h / .c           # AST node structures, printer, destructor
│   ├── parser.h / .c        # Predictive recursive descent parser
│   ├── symbol_table.h / .c  # Scoped symbol table with parent links
│   ├── semantic.h / .c      # Type validation and semantic checking
│   ├── tac.h / .c           # Structured Three-Address Code representation
│   └── optimizer.h / .c     # Multi-pass peephole optimizer
│
├── examples/                # Example MiniC source files
│   ├── basic.mc             # Primary academic demo (declarations, while loop)
│   ├── arithmetic.mc        # Precedence and parenthetical grouping
│   ├── control_flow.mc      # if-else branching and block scoping
│   ├── optimization.mc      # Showcase of all 6 optimization passes
│   ├── nested_expression.mc # Float calculations and nested expressions
│   └── errors.mc            # Demonstration of diagnostic error reporting
│
├── tests/                   # Automated test harness and unit test files
│   ├── test_runner.c        # In-process test harness (54 assertions)
│   ├── test_lexer.mc
│   ├── test_parser.mc
│   ├── test_semantic.mc
│   ├── test_tac.mc
│   ├── test_optimization.mc
│   ├── test_while.mc
│   └── test_errors.mc
│
└── docs/                    # University documentation deliverables
    ├── DOCUMENTATION.md     # 11-Chapter Academic Report & Lab Manual
    ├── VIVA_QUESTIONS.md    # 45 Comprehensive Viva Voce Q&As
    └── PRESENTATION_CONTENT.md # 15-Slide Presentation Deck Outline
```

---

## 7. Requirements & Prerequisites
- **Operating System**: Windows (MinGW / MSYS2), Linux (GCC), or macOS (Clang/GCC).
- **C Compiler**: Standard C compiler supporting C11 (`gcc` or `clang`).
- **Build Tool**: `make` or `mingw32-make`.

---

## 8. Build Instructions

### Compile Executable
```bash
# On Linux / macOS:
make

# On Windows (MinGW):
mingw32-make
```
This builds the binary `minic` (or `minic.exe` on Windows) with strict compiler flags (`-std=c11 -Wall -Wextra -Wpedantic -O2`).

### Clean Build Artifacts
```bash
make clean
# or on Windows:
mingw32-make clean
```

---

## 9. Run Instructions

### 1. Compile a MiniC Source File (Default: Shows TAC, Optimized TAC, Report)
```bash
./minic examples/basic.mc
```

### 2. Inspect Specific Compiler Stages
```bash
# Print Token Stream:
./minic examples/basic.mc --tokens

# Print Abstract Syntax Tree (AST):
./minic examples/basic.mc --ast

# Print Scoped Symbol Table:
./minic examples/basic.mc --symtab

# Print Raw Unoptimized Three-Address Code:
./minic examples/basic.mc --tac

# Print Optimized Three-Address Code:
./minic examples/basic.mc --optimized

# Print Optimization Report:
./minic examples/basic.mc --report

# Print ALL Compiler Stages simultaneously:
./minic examples/basic.mc --all
```

### 3. Interactive Mode
Run the compiler without arguments to access the interactive CLI shell:
```bash
./minic
```
Menu options:
```
========================================
           MiniC Compiler               
========================================
1. Enter source code interactively
2. Compile source file
3. Run demo test case
4. Exit
----------------------------------------
```

---

## 10. Primary Academic Demonstration

### Input Program (`examples/basic.mc`)
```c
int x = 3 + 4;
int y = x + 0;

while (y < 20) {
    y = y + 1;
}
```

### Unoptimized TAC vs. Optimized TAC

```
RAW THREE-ADDRESS CODE                   OPTIMIZED THREE-ADDRESS CODE
----------------------------------       ----------------------------------
    t1     = 3 + 4                           x      = 7
    x      = t1                              y      = 7
    t2     = x + 0                       L1        :
    y      = t2                              t3     = y < 20
L1        :                                  ifFalse t3 goto L2
    t3     = y < 20                          y      = y + 1
    ifFalse t3 goto L2                       goto L1
    t4     = y + 1                       L2        :
    y      = t4
    goto L1
L2        :
```

### Optimization Report Output
```
========================================
OPTIMIZATION REPORT
========================================

Original TAC instructions : 11
Optimized TAC instructions: 8

Instructions removed      : 3

Constant folding          : 1
Algebraic simplification  : 1
Constant propagation      : 1
Temporary elimination     : 3
Dead code elimination     : 0
Unreachable code removal  : 0

Reduction                 : 27.27%
========================================
```

---

## 11. Testing & Validation

### Run Automated Test Suite
```bash
# On Linux / macOS:
make test

# On Windows:
mingw32-make test
```

### Test Suite Execution Output
```
========================================
MiniC Compiler Automated Test Suite
========================================

Lexer tests                    PASS
Parser tests                   PASS
AST tests                      PASS
Symbol table tests             PASS
Semantic tests                 PASS
TAC generation tests           PASS
Optimization tests             PASS
Error handling & boundary      PASS

========================================
Total Assertions : 54
Passed           : 54
Failed           : 0
========================================
```

---

## 12. Peephole Optimization Showcase

Running `./minic examples/optimization.mc` demonstrates all six optimization rules in action:

| Optimization Pass | Code Pattern in Source | Transformation Effect |
| :--- | :--- | :--- |
| **Constant Folding** | `int c1 = 3 + 4;` | Folded at compile time $\to$ `c1 = 7` |
| **Constant Folding** | `int c2 = 10 * 5;` | Folded at compile time $\to$ `c2 = 50` |
| **Algebraic Simplification** | `int s1 = a + 0;` | Simplified using additive identity $\to$ `s1 = 25` |
| **Algebraic Simplification** | `int s2 = a * 1;` | Simplified using multiplicative identity $\to$ `s2 = 25` |
| **Algebraic Simplification** | `int s3 = a * 0;` | Simplified using annihilator property $\to$ `s3 = 0` |
| **Constant Propagation** | `int p1 = 10; int p2 = p1 + 5;` | Propagates 10 into $10+5 \to$ folds `p2 = 15` |
| **Temporary Elimination** | `t1 = 3 + 4; c1 = t1` | Temporary `t1` removed; assigned directly to `c1` |
| **Dead Code Elimination** | `int dead = 100; dead = 200;` | First assignment `dead = 100` eliminated |

**Measured Result**: 18 original instructions reduced to 10 instructions (**44.44% reduction**).

---

## 13. Error Handling Examples

The compiler gracefully detects and diagnoses errors with source coordinates without crashing:

### Lexical Error (`int x = 10 @ 20;`)
```
[Lexical Error] Line 1, Column 12: Unexpected character '@' (ASCII 64)
```

### Syntax Error (`int x = (10 + 20;`)
```
[Syntax Error] Line 1, Column 17: Expected ')' to close parenthesized expression (found ';' of type SEMICOLON)
```

### Semantic Error: Undeclared Variable (`target = 50;`)
```
[Semantic Error] Line 1, Column 1: Assignment to undeclared variable 'target'.
```

### Semantic Error: Duplicate Declaration (`int x; int x;`)
```
[Semantic Error] Line 1, Column 8: Variable 'x' already declared in the current scope (previously at line 1).
```

### Semantic Error: Modulo on Float (`float f = 5.5; int r = f % 2;`)
```
[Semantic Error] Line 1, Column 25: Invalid operands to binary '%' (both operands must be integer).
```

---

## 14. Academic Deliverables & Documentation

Complete academic project reports and presentation resources are located in the `docs/` folder:
- **`docs/DOCUMENTATION.md`**: Full 11-chapter University laboratory project report (Introduction, Problem Statement, Objectives, Literature Survey, System Design with Mermaid Diagrams, Methodology, Compiler Concepts, Implementation, Testing & Results, Conclusion, Future Enhancements, References, and Formal Grammar Appendix).
- **`docs/VIVA_QUESTIONS.md`**: 45 in-depth viva voce questions and answers covering theory, code mechanics, trade-offs, and boundary cases.
- **`docs/PRESENTATION_CONTENT.md`**: 15-slide academic presentation deck outline with speaker script.

---

## 15. Limitations & Future Work

### Current Limitations
- Excludes arrays, pointers, structures, and unions.
- Does not support user-defined functions or recursion.
- Concludes at Three-Address Code (no target assembly/machine code).

### Future Work
- Construct Control-Flow Graphs (CFGs) and Static Single Assignment (SSA) form with $\phi$-functions.
- Global data-flow analysis (available expressions, reaching definitions).
- Back-end target code generation for RISC-V or x86-64 assembly.
