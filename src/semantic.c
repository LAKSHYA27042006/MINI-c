#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void semantic_analyzer_init(SemanticAnalyzer *analyzer, SymbolTable *st, ErrorReporter *reporter) {
    if (!analyzer) return;
    analyzer->symtab = st;
    analyzer->reporter = reporter;
}

static DataType analyze_expr(SemanticAnalyzer *analyzer, ASTNode *node);
static void analyze_statement(SemanticAnalyzer *analyzer, ASTNode *node);

static DataType analyze_expr(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!node) return DATA_TYPE_UNKNOWN;

    switch (node->type) {
        case NODE_INT_LITERAL:
            node->inferred_type = DATA_TYPE_INT;
            return DATA_TYPE_INT;

        case NODE_FLOAT_LITERAL:
            node->inferred_type = DATA_TYPE_FLOAT;
            return DATA_TYPE_FLOAT;

        case NODE_IDENTIFIER: {
            Symbol *sym = symbol_table_lookup(analyzer->symtab, node->data.identifier.name);
            if (!sym) {
                error_report(analyzer->reporter, ERR_SEMANTIC, node->line, node->column,
                             "Variable '%s' is not declared.", node->data.identifier.name);
                node->inferred_type = DATA_TYPE_UNKNOWN;
                return DATA_TYPE_UNKNOWN;
            }

            if (!sym->is_initialized) {
                error_report(analyzer->reporter, ERR_WARNING, node->line, node->column,
                             "Variable '%s' may be used uninitialized.", node->data.identifier.name);
            }

            node->inferred_type = sym->type;
            return sym->type;
        }

        case NODE_UNARY_EXPR: {
            DataType operand_type = analyze_expr(analyzer, node->data.unary_expr.operand);
            node->inferred_type = operand_type;
            return operand_type;
        }

        case NODE_BINARY_EXPR: {
            DataType left_type = analyze_expr(analyzer, node->data.binary_expr.left);
            DataType right_type = analyze_expr(analyzer, node->data.binary_expr.right);

            BinaryOp op = node->data.binary_expr.op;

            /* Check relational operations */
            if (op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE || op == OP_EQ || op == OP_NE) {
                if (left_type == DATA_TYPE_UNKNOWN || right_type == DATA_TYPE_UNKNOWN) {
                    node->inferred_type = DATA_TYPE_INT;
                    return DATA_TYPE_INT;
                }
                /* Relational operations yield int boolean 0/1 */
                node->inferred_type = DATA_TYPE_INT;
                return DATA_TYPE_INT;
            }

            /* Modulo requires integer operands */
            if (op == OP_MOD) {
                if (left_type == DATA_TYPE_FLOAT || right_type == DATA_TYPE_FLOAT) {
                    error_report(analyzer->reporter, ERR_SEMANTIC, node->line, node->column,
                                 "Invalid operands to binary '%%' (both operands must be integer).");
                }
                node->inferred_type = DATA_TYPE_INT;
                return DATA_TYPE_INT;
            }

            /* Standard arithmetic */
            if (left_type == DATA_TYPE_FLOAT || right_type == DATA_TYPE_FLOAT) {
                node->inferred_type = DATA_TYPE_FLOAT;
                return DATA_TYPE_FLOAT;
            }

            node->inferred_type = DATA_TYPE_INT;
            return DATA_TYPE_INT;
        }

        default:
            return DATA_TYPE_UNKNOWN;
    }
}

static void analyze_statement(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_VAR_DECL: {
            const char *name = node->data.var_decl.var_name;
            DataType type = node->data.var_decl.var_type;

            Symbol *existing = symbol_table_lookup_current(analyzer->symtab, name);
            if (existing) {
                error_report(analyzer->reporter, ERR_SEMANTIC, node->line, node->column,
                             "Variable '%s' already declared in the current scope (previously at line %d).",
                             name, existing->line);
                return;
            }

            symbol_table_insert(analyzer->symtab, name, type, node->line, node->column);
            Symbol *new_sym = symbol_table_lookup_current(analyzer->symtab, name);

            if (node->data.var_decl.init_expr) {
                DataType init_type = analyze_expr(analyzer, node->data.var_decl.init_expr);
                if (new_sym) {
                    new_sym->is_initialized = true;
                }

                if (type == DATA_TYPE_INT && init_type == DATA_TYPE_FLOAT) {
                    error_report(analyzer->reporter, ERR_WARNING, node->line, node->column,
                                 "Conversion from 'float' to 'int' in initialization of '%s' may alter value.",
                                 name);
                }
            }
            break;
        }

        case NODE_ASSIGN: {
            const char *name = node->data.assign.var_name;
            Symbol *sym = symbol_table_lookup(analyzer->symtab, name);

            if (!sym) {
                error_report(analyzer->reporter, ERR_SEMANTIC, node->line, node->column,
                             "Assignment to undeclared variable '%s'.", name);
            }

            DataType val_type = analyze_expr(analyzer, node->data.assign.expr);

            if (sym) {
                sym->is_initialized = true;

                if (sym->type == DATA_TYPE_INT && val_type == DATA_TYPE_FLOAT) {
                    error_report(analyzer->reporter, ERR_WARNING, node->line, node->column,
                                 "Conversion from 'float' to 'int' in assignment to '%s' may alter value.",
                                 name);
                }
            }
            break;
        }

        case NODE_IF: {
            analyze_expr(analyzer, node->data.if_stmt.condition);
            analyze_statement(analyzer, node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch) {
                analyze_statement(analyzer, node->data.if_stmt.else_branch);
            }
            break;
        }

        case NODE_WHILE: {
            analyze_expr(analyzer, node->data.while_stmt.condition);
            analyze_statement(analyzer, node->data.while_stmt.body);
            break;
        }

        case NODE_BLOCK: {
            symbol_table_enter_scope(analyzer->symtab);
            for (size_t i = 0; i < node->data.block.stmt_count; i++) {
                analyze_statement(analyzer, node->data.block.statements[i]);
            }
            symbol_table_exit_scope(analyzer->symtab);
            break;
        }

        case NODE_PROGRAM: {
            for (size_t i = 0; i < node->data.program.stmt_count; i++) {
                analyze_statement(analyzer, node->data.program.statements[i]);
            }
            break;
        }

        default:
            break;
    }
}

bool semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *root) {
    if (!analyzer || !root) return false;
    analyze_statement(analyzer, root);
    return !error_reporter_has_errors(analyzer->reporter);
}
