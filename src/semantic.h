#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"
#include "error.h"

typedef struct {
    SymbolTable *symtab;
    ErrorReporter *reporter;
} SemanticAnalyzer;

/* Initialize semantic analyzer */
void semantic_analyzer_init(SemanticAnalyzer *analyzer, SymbolTable *st, ErrorReporter *reporter);

/* Perform semantic checks on AST */
bool semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *root);

#endif /* SEMANTIC_H */
