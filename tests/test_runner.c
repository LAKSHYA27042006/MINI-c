#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "error.h"
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol_table.h"
#include "semantic.h"
#include "tac.h"
#include "optimizer.h"

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_tests_run++; \
    if (cond) { \
        g_tests_passed++; \
    } else { \
        g_tests_failed++; \
        printf("  FAILED: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
    } \
} while (0)

/* Helper: tokenize source string */
static TokenArray run_lexer(const char *source, ErrorReporter *rep) {
    Lexer lexer;
    lexer_init(&lexer, source, rep);
    return lexer_tokenize(&lexer);
}

/* Helper: parse source string to AST */
static ASTNode *run_parser(const char *source, ErrorReporter *rep) {
    TokenArray tokens = run_lexer(source, rep);
    Parser parser;
    parser_init(&parser, &tokens, rep);
    ASTNode *root = parser_parse_program(&parser);
    token_array_free(&tokens);
    return root;
}

/* Helper: full compile to raw TAC */
static TACProgram run_to_tac(const char *source, ErrorReporter *rep) {
    TACProgram prog;
    tac_program_init(&prog);
    ASTNode *root = run_parser(source, rep);
    if (!error_reporter_has_errors(rep) && root) {
        SymbolTable st;
        symbol_table_init(&st);
        SemanticAnalyzer sem;
        semantic_analyzer_init(&sem, &st, rep);
        semantic_analyze(&sem, root);
        if (!error_reporter_has_errors(rep)) {
            tac_generate(&prog, root);
        }
        symbol_table_free(&st);
    }
    if (root) ast_free(root);
    return prog;
}

/* ============================================================
 * 1. LEXER TESTS
 * ============================================================ */
static bool test_lexer_suite(void) {
    int start_failed = g_tests_failed;

    /* 1.1 Keywords and Identifiers */
    ErrorReporter rep;
    error_reporter_init(&rep);
    TokenArray t1 = run_lexer("int float if else while my_var_1", &rep);
    TEST_ASSERT(t1.count == 7, "Lexer: Correct token count for keywords/identifiers");
    TEST_ASSERT(t1.tokens[0].type == TOKEN_KW_INT, "Lexer: Match 'int'");
    TEST_ASSERT(t1.tokens[1].type == TOKEN_KW_FLOAT, "Lexer: Match 'float'");
    TEST_ASSERT(t1.tokens[2].type == TOKEN_KW_IF, "Lexer: Match 'if'");
    TEST_ASSERT(t1.tokens[3].type == TOKEN_KW_ELSE, "Lexer: Match 'else'");
    TEST_ASSERT(t1.tokens[4].type == TOKEN_KW_WHILE, "Lexer: Match 'while'");
    TEST_ASSERT(t1.tokens[5].type == TOKEN_IDENTIFIER, "Lexer: Match identifier 'my_var_1'");
    TEST_ASSERT(t1.tokens[6].type == TOKEN_EOF, "Lexer: Match EOF token");
    token_array_free(&t1);
    error_reporter_free(&rep);

    /* 1.2 Integer and Float Literals */
    error_reporter_init(&rep);
    TokenArray t2 = run_lexer("42 3.14159", &rep);
    TEST_ASSERT(t2.tokens[0].type == TOKEN_INT_LITERAL && t2.tokens[0].value.int_val == 42,
                "Lexer: Parse integer literal value 42");
    TEST_ASSERT(t2.tokens[1].type == TOKEN_FLOAT_LITERAL,
                "Lexer: Parse float literal token");
    token_array_free(&t2);
    error_reporter_free(&rep);

    /* 1.3 Comments (Single-line and Multi-line) */
    error_reporter_init(&rep);
    TokenArray t3 = run_lexer("int x; // this is a comment\n/* multi\nline */ float y;", &rep);
    TEST_ASSERT(!error_reporter_has_errors(&rep), "Lexer: Comments skipped cleanly without errors");
    TEST_ASSERT(t3.tokens[0].type == TOKEN_KW_INT, "Lexer: Token after comments");
    TEST_ASSERT(t3.tokens[3].type == TOKEN_KW_FLOAT, "Lexer: Token after multi-line comment");
    token_array_free(&t3);
    error_reporter_free(&rep);

    /* 1.4 Lexical Error on Invalid Character */
    error_reporter_init(&rep);
    TokenArray t4 = run_lexer("int x = 10 @ 20;", &rep);
    TEST_ASSERT(error_reporter_has_errors(&rep), "Lexer: Detect invalid character '@'");
    TEST_ASSERT(rep.head && rep.head->type == ERR_LEXICAL, "Lexer: Reported error is ERR_LEXICAL");
    token_array_free(&t4);
    error_reporter_free(&rep);

    return (g_tests_failed == start_failed);
}

/* ============================================================
 * 2. PARSER & AST TESTS
 * ============================================================ */
static bool test_parser_suite(void) {
    int start_failed = g_tests_failed;

    /* 2.1 Operator Precedence (* over +) */
    ErrorReporter rep;
    error_reporter_init(&rep);
    ASTNode *ast1 = run_parser("int res = 2 + 3 * 4;", &rep);
    TEST_ASSERT(!error_reporter_has_errors(&rep), "Parser: Parse '2 + 3 * 4;' without error");
    TEST_ASSERT(ast1 && ast1->data.program.stmt_count == 1, "Parser: Program has 1 statement");
    ASTNode *decl1 = ast1->data.program.statements[0];
    TEST_ASSERT(decl1->type == NODE_VAR_DECL, "Parser: Statement is VarDecl");
    ASTNode *init1 = decl1->data.var_decl.init_expr;
    TEST_ASSERT(init1->type == NODE_BINARY_EXPR && init1->data.binary_expr.op == OP_ADD,
                "Parser: Root of 2 + 3 * 4 is '+'");
    TEST_ASSERT(init1->data.binary_expr.right->type == NODE_BINARY_EXPR &&
                init1->data.binary_expr.right->data.binary_expr.op == OP_MUL,
                "Parser: Right child is '3 * 4' ('*' has higher precedence)");
    ast_free(ast1);
    error_reporter_free(&rep);

    /* 2.2 Parentheses Overriding Precedence */
    error_reporter_init(&rep);
    ASTNode *ast2 = run_parser("int res = (2 + 3) * 4;", &rep);
    TEST_ASSERT(!error_reporter_has_errors(&rep), "Parser: Parse '(2 + 3) * 4;'");
    ASTNode *init2 = ast2->data.program.statements[0]->data.var_decl.init_expr;
    TEST_ASSERT(init2->type == NODE_BINARY_EXPR && init2->data.binary_expr.op == OP_MUL,
                "Parser: Root of (2 + 3) * 4 is '*'");
    TEST_ASSERT(init2->data.binary_expr.left->type == NODE_BINARY_EXPR &&
                init2->data.binary_expr.left->data.binary_expr.op == OP_ADD,
                "Parser: Left child is '(2 + 3)' grouped by parentheses");
    ast_free(ast2);
    error_reporter_free(&rep);

    /* 2.3 If-Else Structure */
    error_reporter_init(&rep);
    ASTNode *ast3 = run_parser("if (x > 0) { y = 1; } else { y = 0; }", &rep);
    TEST_ASSERT(ast3 && ast3->data.program.stmt_count == 1, "Parser: If-Else parsed");
    ASTNode *if_node = ast3->data.program.statements[0];
    TEST_ASSERT(if_node->type == NODE_IF, "Parser: Node type is NODE_IF");
    TEST_ASSERT(if_node->data.if_stmt.else_branch != NULL, "Parser: Else branch is populated");
    ast_free(ast3);
    error_reporter_free(&rep);

    /* 2.4 While Loop Structure */
    error_reporter_init(&rep);
    ASTNode *ast4 = run_parser("while (count < 10) { count = count + 1; }", &rep);
    TEST_ASSERT(ast4 && ast4->data.program.stmt_count == 1, "Parser: While parsed");
    ASTNode *while_node = ast4->data.program.statements[0];
    TEST_ASSERT(while_node->type == NODE_WHILE, "Parser: Node type is NODE_WHILE");
    ast_free(ast4);
    error_reporter_free(&rep);

    return (g_tests_failed == start_failed);
}

/* ============================================================
 * 3. SYMBOL TABLE TESTS
 * ============================================================ */
static bool test_symbol_table_suite(void) {
    int start_failed = g_tests_failed;

    SymbolTable st;
    symbol_table_init(&st);

    /* 3.1 Global Insert and Lookup */
    bool ok1 = symbol_table_insert(&st, "global_var", DATA_TYPE_INT, 1, 1);
    TEST_ASSERT(ok1, "SymbolTable: Insert 'global_var' in global scope");
    Symbol *s1 = symbol_table_lookup(&st, "global_var");
    TEST_ASSERT(s1 != NULL && s1->type == DATA_TYPE_INT, "SymbolTable: Lookup 'global_var'");

    /* 3.2 Duplicate Insert in Same Scope Detected */
    bool ok2 = symbol_table_insert(&st, "global_var", DATA_TYPE_FLOAT, 2, 1);
    TEST_ASSERT(!ok2, "SymbolTable: Detect duplicate insert in same scope");

    /* 3.3 Scoped Block Insertion & Shadowing */
    symbol_table_enter_scope(&st);
    bool ok3 = symbol_table_insert(&st, "global_var", DATA_TYPE_FLOAT, 3, 5);
    TEST_ASSERT(ok3, "SymbolTable: Allow shadowing 'global_var' in nested scope");
    Symbol *s_inner = symbol_table_lookup(&st, "global_var");
    TEST_ASSERT(s_inner != NULL && s_inner->type == DATA_TYPE_FLOAT,
                "SymbolTable: Shadowed symbol resolved to inner type (float)");

    /* 3.4 Scope Exit Restores Outer Scope */
    symbol_table_exit_scope(&st);
    Symbol *s_outer = symbol_table_lookup(&st, "global_var");
    TEST_ASSERT(s_outer != NULL && s_outer->type == DATA_TYPE_INT,
                "SymbolTable: Scope exit restores outer symbol type (int)");

    symbol_table_free(&st);
    return (g_tests_failed == start_failed);
}

/* ============================================================
 * 4. SEMANTIC ANALYSIS TESTS
 * ============================================================ */
static bool test_semantic_suite(void) {
    int start_failed = g_tests_failed;

    /* 4.1 Valid Program */
    ErrorReporter rep1;
    error_reporter_init(&rep1);
    ASTNode *ast1 = run_parser("int a = 10; float b = 20.5; float c = a + b;", &rep1);
    SymbolTable st1;
    symbol_table_init(&st1);
    SemanticAnalyzer sem1;
    semantic_analyzer_init(&sem1, &st1, &rep1);
    bool sem_ok1 = semantic_analyze(&sem1, ast1);
    TEST_ASSERT(sem_ok1 && !error_reporter_has_errors(&rep1), "Semantic: Valid declarations and arithmetic");
    symbol_table_free(&st1);
    ast_free(ast1);
    error_reporter_free(&rep1);

    /* 4.2 Undeclared Variable Detection */
    ErrorReporter rep2;
    error_reporter_init(&rep2);
    ASTNode *ast2 = run_parser("x = 100;", &rep2);
    SymbolTable st2;
    symbol_table_init(&st2);
    SemanticAnalyzer sem2;
    semantic_analyzer_init(&sem2, &st2, &rep2);
    semantic_analyze(&sem2, ast2);
    TEST_ASSERT(error_reporter_has_errors(&rep2), "Semantic: Catch undeclared variable 'x'");
    symbol_table_free(&st2);
    ast_free(ast2);
    error_reporter_free(&rep2);

    /* 4.3 Duplicate Declaration Detection */
    ErrorReporter rep3;
    error_reporter_init(&rep3);
    ASTNode *ast3 = run_parser("int x; int x;", &rep3);
    SymbolTable st3;
    symbol_table_init(&st3);
    SemanticAnalyzer sem3;
    semantic_analyzer_init(&sem3, &st3, &rep3);
    semantic_analyze(&sem3, ast3);
    TEST_ASSERT(error_reporter_has_errors(&rep3), "Semantic: Catch duplicate declaration of 'x'");
    symbol_table_free(&st3);
    ast_free(ast3);
    error_reporter_free(&rep3);

    /* 4.4 Modulo on Float Detection */
    ErrorReporter rep4;
    error_reporter_init(&rep4);
    ASTNode *ast4 = run_parser("float f = 5.5; int rem = f % 2;", &rep4);
    SymbolTable st4;
    symbol_table_init(&st4);
    SemanticAnalyzer sem4;
    semantic_analyzer_init(&sem4, &st4, &rep4);
    semantic_analyze(&sem4, ast4);
    TEST_ASSERT(error_reporter_has_errors(&rep4), "Semantic: Catch modulo operator on float operand");
    symbol_table_free(&st4);
    ast_free(ast4);
    error_reporter_free(&rep4);

    return (g_tests_failed == start_failed);
}

/* ============================================================
 * 5. TAC GENERATION TESTS
 * ============================================================ */
static bool test_tac_suite(void) {
    int start_failed = g_tests_failed;

    ErrorReporter rep;
    error_reporter_init(&rep);
    const char *src =
        "int x = 10;\n"
        "int y = 20;\n"
        "int z = x + y * 2;\n";
    TACProgram prog = run_to_tac(src, &rep);
    TEST_ASSERT(!error_reporter_has_errors(&rep), "TAC: Generate TAC without errors");
    TEST_ASSERT(prog.count > 0, "TAC: Instructions emitted");

    /* Verify instruction sequence contains temporary multiplication then addition */
    bool found_mul = false;
    bool found_add = false;
    TACInstr *curr = prog.head;
    while (curr) {
        if (curr->op == TAC_MUL) found_mul = true;
        if (curr->op == TAC_ADD) found_add = true;
        curr = curr->next;
    }
    TEST_ASSERT(found_mul && found_add, "TAC: Contains both TAC_MUL and TAC_ADD instructions");

    tac_program_free(&prog);
    error_reporter_free(&rep);

    return (g_tests_failed == start_failed);
}

/* ============================================================
 * 6. PEEPHOLE OPTIMIZATION TESTS
 * ============================================================ */
static bool test_optimization_suite(void) {
    int start_failed = g_tests_failed;

    /* 6.1 Constant Folding */
    ErrorReporter rep1;
    error_reporter_init(&rep1);
    TACProgram p1 = run_to_tac("int x = 3 + 4;", &rep1);
    OptimizationStats s1;
    TACProgram *opt1 = optimize_tac(&p1, &s1);
    TEST_ASSERT(s1.constant_folding_count >= 1, "Optimizer: Constant folding applied to 3 + 4");
    /* The result instruction should assign 7 */
    bool has_7 = false;
    TACInstr *c1 = opt1->head;
    while (c1) {
        if (c1->arg1.type == OPERAND_CONST_INT && c1->arg1.val.int_val == 7) {
            has_7 = true;
        }
        c1 = c1->next;
    }
    TEST_ASSERT(has_7, "Optimizer: Value 7 generated after folding");
    tac_program_free(opt1);
    free(opt1);
    tac_program_free(&p1);
    error_reporter_free(&rep1);

    /* 6.2 Algebraic Simplification */
    ErrorReporter rep2;
    error_reporter_init(&rep2);
    TACProgram p2 = run_to_tac("int a = 10; int b = a + 0; int c = a * 1; int d = a * 0;", &rep2);
    OptimizationStats s2;
    TACProgram *opt2 = optimize_tac(&p2, &s2);
    TEST_ASSERT(s2.algebraic_simplification_count > 0 || s2.constant_propagation_count > 0,
                "Optimizer: Algebraic simplification or propagation triggered");
    tac_program_free(opt2);
    free(opt2);
    tac_program_free(&p2);
    error_reporter_free(&rep2);

    /* 6.3 Constant Propagation */
    ErrorReporter rep3;
    error_reporter_init(&rep3);
    TACProgram p3 = run_to_tac("int x = 10; int y = x + 5;", &rep3);
    OptimizationStats s3;
    TACProgram *opt3 = optimize_tac(&p3, &s3);
    TEST_ASSERT(s3.constant_propagation_count >= 1, "Optimizer: Constant propagation (x -> 10)");
    tac_program_free(opt3);
    free(opt3);
    tac_program_free(&p3);
    error_reporter_free(&rep3);

    /* 6.4 Dead Code Elimination (Consecutive overwrite) */
    ErrorReporter rep4;
    error_reporter_init(&rep4);
    TACProgram p4 = run_to_tac("int x = 10; x = 20;", &rep4);
    OptimizationStats s4;
    TACProgram *opt4 = optimize_tac(&p4, &s4);
    TEST_ASSERT(s4.dead_code_count >= 1, "Optimizer: Dead store to x eliminated");
    tac_program_free(opt4);
    free(opt4);
    tac_program_free(&p4);
    error_reporter_free(&rep4);

    /* 6.5 Safe Division by Zero Check (Must NOT crash or fold) */
    ErrorReporter rep5;
    error_reporter_init(&rep5);
    TACProgram p5 = run_to_tac("int x = 10 / 0;", &rep5);
    OptimizationStats s5;
    TACProgram *opt5 = optimize_tac(&p5, &s5);
    TEST_ASSERT(s5.constant_folding_count == 0, "Optimizer: Division by zero is safely NOT folded");
    tac_program_free(opt5);
    free(opt5);
    tac_program_free(&p5);
    error_reporter_free(&rep5);

    return (g_tests_failed == start_failed);
}

/* ============================================================
 * 7. ERROR HANDLING & BOUNDARY TESTS
 * ============================================================ */
static bool test_error_and_boundary_suite(void) {
    int start_failed = g_tests_failed;

    /* 7.1 Missing Semicolon */
    ErrorReporter rep1;
    error_reporter_init(&rep1);
    ASTNode *a1 = run_parser("int x = 10", &rep1);
    TEST_ASSERT(error_reporter_has_errors(&rep1), "Error Handling: Catch missing semicolon");
    if (a1) ast_free(a1);
    error_reporter_free(&rep1);

    /* 7.2 Missing Closing Parenthesis */
    ErrorReporter rep2;
    error_reporter_init(&rep2);
    ASTNode *a2 = run_parser("int x = (10 + 20;", &rep2);
    TEST_ASSERT(error_reporter_has_errors(&rep2), "Error Handling: Catch missing closing parenthesis");
    if (a2) ast_free(a2);
    error_reporter_free(&rep2);

    /* 7.3 Empty File Handling */
    ErrorReporter rep3;
    error_reporter_init(&rep3);
    ASTNode *a3 = run_parser("", &rep3);
    TEST_ASSERT(!error_reporter_has_errors(&rep3), "Boundary: Empty source parsed without error");
    TEST_ASSERT(a3 && a3->data.program.stmt_count == 0, "Boundary: Empty source has 0 statements");
    if (a3) ast_free(a3);
    error_reporter_free(&rep3);

    /* 7.4 Deeply Nested Parentheses */
    ErrorReporter rep4;
    error_reporter_init(&rep4);
    ASTNode *a4 = run_parser("int x = ((((1 + 2) + 3) + 4) + 5);", &rep4);
    TEST_ASSERT(!error_reporter_has_errors(&rep4), "Boundary: Deeply nested parentheses parsed cleanly");
    if (a4) ast_free(a4);
    error_reporter_free(&rep4);

    /* 7.5 Deeply Nested Blocks */
    ErrorReporter rep5;
    error_reporter_init(&rep5);
    ASTNode *a5 = run_parser("int a = 1; { int b = 2; { int c = 3; { int d = 4; } } }", &rep5);
    TEST_ASSERT(!error_reporter_has_errors(&rep5), "Boundary: Deeply nested blocks parsed cleanly");
    SymbolTable st5;
    symbol_table_init(&st5);
    SemanticAnalyzer sem5;
    semantic_analyzer_init(&sem5, &st5, &rep5);
    bool ok5 = semantic_analyze(&sem5, a5);
    TEST_ASSERT(ok5 && !error_reporter_has_errors(&rep5), "Boundary: Deeply nested blocks pass semantic analysis");
    symbol_table_free(&st5);
    if (a5) ast_free(a5);
    error_reporter_free(&rep5);

    return (g_tests_failed == start_failed);
}

/* ============================================================
 * MAIN TEST SUITE DRIVER
 * ============================================================ */
int main(void) {
    printf("========================================\n");
    printf("MiniC Compiler Automated Test Suite\n");
    printf("========================================\n\n");

    bool r_lex = test_lexer_suite();
    printf("%-30s %s\n", "Lexer tests", r_lex ? "PASS" : "FAIL");

    bool r_par = test_parser_suite();
    printf("%-30s %s\n", "Parser tests", r_par ? "PASS" : "FAIL");

    bool r_ast = r_par; // Verified together in parser suite
    printf("%-30s %s\n", "AST tests", r_ast ? "PASS" : "FAIL");

    bool r_sym = test_symbol_table_suite();
    printf("%-30s %s\n", "Symbol table tests", r_sym ? "PASS" : "FAIL");

    bool r_sem = test_semantic_suite();
    printf("%-30s %s\n", "Semantic tests", r_sem ? "PASS" : "FAIL");

    bool r_tac = test_tac_suite();
    printf("%-30s %s\n", "TAC generation tests", r_tac ? "PASS" : "FAIL");

    bool r_opt = test_optimization_suite();
    printf("%-30s %s\n", "Optimization tests", r_opt ? "PASS" : "FAIL");

    bool r_err = test_error_and_boundary_suite();
    printf("%-30s %s\n", "Error handling & boundary", r_err ? "PASS" : "FAIL");

    printf("\n========================================\n");
    printf("Total Assertions : %d\n", g_tests_run);
    printf("Passed           : %d\n", g_tests_passed);
    printf("Failed           : %d\n", g_tests_failed);
    printf("========================================\n");

    return (g_tests_failed == 0) ? 0 : 1;
}
