#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *safe_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *res = (char *)malloc(len + 1);
    if (!res) {
        fprintf(stderr, "Fatal: Out of memory in safe_strdup\n");
        exit(EXIT_FAILURE);
    }
    memcpy(res, s, len + 1);
    return res;
}

const char *data_type_name(DataType type) {
    switch (type) {
        case DATA_TYPE_INT:     return "int";
        case DATA_TYPE_FLOAT:   return "float";
        case DATA_TYPE_UNKNOWN: return "unknown";
        default:                return "invalid";
    }
}

const char *binary_op_name(BinaryOp op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_LT:  return "<";
        case OP_GT:  return ">";
        case OP_LE:  return "<=";
        case OP_GE:  return ">=";
        case OP_EQ:  return "==";
        case OP_NE:  return "!=";
        default:     return "?";
    }
}

const char *ast_node_type_name(ASTNodeType type) {
    switch (type) {
        case NODE_PROGRAM:      return "Program";
        case NODE_VAR_DECL:     return "VarDecl";
        case NODE_ASSIGN:       return "Assignment";
        case NODE_BINARY_EXPR:  return "BinaryExpr";
        case NODE_UNARY_EXPR:   return "UnaryExpr";
        case NODE_INT_LITERAL:  return "IntLiteral";
        case NODE_FLOAT_LITERAL:return "FloatLiteral";
        case NODE_IDENTIFIER:   return "Identifier";
        case NODE_IF:           return "IfStatement";
        case NODE_WHILE:        return "WhileStatement";
        case NODE_BLOCK:        return "Block";
        default:                return "UnknownNode";
    }
}

static ASTNode *alloc_node(ASTNodeType type, int line, int col) {
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Fatal: Out of memory allocating ASTNode\n");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->line = line;
    node->column = col;
    node->inferred_type = DATA_TYPE_UNKNOWN;
    return node;
}

ASTNode *ast_create_program(void) {
    ASTNode *node = alloc_node(NODE_PROGRAM, 1, 1);
    node->data.program.stmt_capacity = 8;
    node->data.program.stmt_count = 0;
    node->data.program.statements = (ASTNode **)malloc(sizeof(ASTNode *) * node->data.program.stmt_capacity);
    if (!node->data.program.statements) {
        fprintf(stderr, "Fatal: Out of memory in ast_create_program\n");
        exit(EXIT_FAILURE);
    }
    return node;
}

void ast_program_add_stmt(ASTNode *prog, ASTNode *stmt) {
    if (!prog || !stmt) return;
    if (prog->type != NODE_PROGRAM) return;

    if (prog->data.program.stmt_count >= prog->data.program.stmt_capacity) {
        prog->data.program.stmt_capacity *= 2;
        ASTNode **new_stmts = (ASTNode **)realloc(prog->data.program.statements,
                                                   sizeof(ASTNode *) * prog->data.program.stmt_capacity);
        if (!new_stmts) {
            fprintf(stderr, "Fatal: Out of memory in ast_program_add_stmt\n");
            exit(EXIT_FAILURE);
        }
        prog->data.program.statements = new_stmts;
    }
    prog->data.program.statements[prog->data.program.stmt_count++] = stmt;
}

ASTNode *ast_create_var_decl(DataType type, const char *name, ASTNode *init_expr, int line, int col) {
    ASTNode *node = alloc_node(NODE_VAR_DECL, line, col);
    node->data.var_decl.var_type = type;
    node->data.var_decl.var_name = safe_strdup(name);
    node->data.var_decl.init_expr = init_expr;
    node->inferred_type = type;
    return node;
}

ASTNode *ast_create_assign(const char *name, ASTNode *expr, int line, int col) {
    ASTNode *node = alloc_node(NODE_ASSIGN, line, col);
    node->data.assign.var_name = safe_strdup(name);
    node->data.assign.expr = expr;
    return node;
}

ASTNode *ast_create_binary(BinaryOp op, ASTNode *left, ASTNode *right, int line, int col) {
    ASTNode *node = alloc_node(NODE_BINARY_EXPR, line, col);
    node->data.binary_expr.op = op;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    return node;
}

ASTNode *ast_create_unary(UnaryOp op, ASTNode *operand, int line, int col) {
    ASTNode *node = alloc_node(NODE_UNARY_EXPR, line, col);
    node->data.unary_expr.op = op;
    node->data.unary_expr.operand = operand;
    return node;
}

ASTNode *ast_create_int_literal(long long val, int line, int col) {
    ASTNode *node = alloc_node(NODE_INT_LITERAL, line, col);
    node->data.int_literal.value = val;
    node->inferred_type = DATA_TYPE_INT;
    return node;
}

ASTNode *ast_create_float_literal(double val, int line, int col) {
    ASTNode *node = alloc_node(NODE_FLOAT_LITERAL, line, col);
    node->data.float_literal.value = val;
    node->inferred_type = DATA_TYPE_FLOAT;
    return node;
}

ASTNode *ast_create_identifier(const char *name, int line, int col) {
    ASTNode *node = alloc_node(NODE_IDENTIFIER, line, col);
    node->data.identifier.name = safe_strdup(name);
    return node;
}

ASTNode *ast_create_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int line, int col) {
    ASTNode *node = alloc_node(NODE_IF, line, col);
    node->data.if_stmt.condition = cond;
    node->data.if_stmt.then_branch = then_b;
    node->data.if_stmt.else_branch = else_b;
    return node;
}

ASTNode *ast_create_while(ASTNode *cond, ASTNode *body, int line, int col) {
    ASTNode *node = alloc_node(NODE_WHILE, line, col);
    node->data.while_stmt.condition = cond;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode *ast_create_block(int line, int col) {
    ASTNode *node = alloc_node(NODE_BLOCK, line, col);
    node->data.block.stmt_capacity = 4;
    node->data.block.stmt_count = 0;
    node->data.block.statements = (ASTNode **)malloc(sizeof(ASTNode *) * node->data.block.stmt_capacity);
    if (!node->data.block.statements) {
        fprintf(stderr, "Fatal: Out of memory in ast_create_block\n");
        exit(EXIT_FAILURE);
    }
    return node;
}

void ast_block_add_stmt(ASTNode *block, ASTNode *stmt) {
    if (!block || !stmt) return;
    if (block->type != NODE_BLOCK) return;

    if (block->data.block.stmt_count >= block->data.block.stmt_capacity) {
        block->data.block.stmt_capacity *= 2;
        ASTNode **new_stmts = (ASTNode **)realloc(block->data.block.statements,
                                                   sizeof(ASTNode *) * block->data.block.stmt_capacity);
        if (!new_stmts) {
            fprintf(stderr, "Fatal: Out of memory in ast_block_add_stmt\n");
            exit(EXIT_FAILURE);
        }
        block->data.block.statements = new_stmts;
    }
    block->data.block.statements[block->data.block.stmt_count++] = stmt;
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void ast_print(const ASTNode *node, int indent) {
    if (!node) {
        print_indent(indent);
        printf("(null)\n");
        return;
    }

    print_indent(indent);

    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program (statements: %zu)\n", node->data.program.stmt_count);
            for (size_t i = 0; i < node->data.program.stmt_count; i++) {
                ast_print(node->data.program.statements[i], indent + 1);
            }
            break;

        case NODE_VAR_DECL:
            printf("VarDecl: %s %s [Line %d]\n",
                   data_type_name(node->data.var_decl.var_type),
                   node->data.var_decl.var_name,
                   node->line);
            if (node->data.var_decl.init_expr) {
                print_indent(indent + 1);
                printf("InitExpr:\n");
                ast_print(node->data.var_decl.init_expr, indent + 2);
            }
            break;

        case NODE_ASSIGN:
            printf("Assignment: %s = [Line %d]\n", node->data.assign.var_name, node->line);
            ast_print(node->data.assign.expr, indent + 1);
            break;

        case NODE_BINARY_EXPR:
            printf("BinaryOp ('%s') [Type: %s]\n",
                   binary_op_name(node->data.binary_expr.op),
                   data_type_name(node->inferred_type));
            ast_print(node->data.binary_expr.left, indent + 1);
            ast_print(node->data.binary_expr.right, indent + 1);
            break;

        case NODE_UNARY_EXPR:
            printf("UnaryOp ('%s')\n", (node->data.unary_expr.op == UNARY_NEG) ? "-" : "+");
            ast_print(node->data.unary_expr.operand, indent + 1);
            break;

        case NODE_INT_LITERAL:
            printf("IntLiteral: %ld\n", (long)node->data.int_literal.value);
            break;

        case NODE_FLOAT_LITERAL:
            printf("FloatLiteral: %g\n", node->data.float_literal.value);
            break;

        case NODE_IDENTIFIER:
            printf("Identifier: '%s' [Type: %s]\n",
                   node->data.identifier.name,
                   data_type_name(node->inferred_type));
            break;

        case NODE_IF:
            printf("IfStatement [Line %d]\n", node->line);
            print_indent(indent + 1);
            printf("Condition:\n");
            ast_print(node->data.if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("ThenBranch:\n");
            ast_print(node->data.if_stmt.then_branch, indent + 2);
            if (node->data.if_stmt.else_branch) {
                print_indent(indent + 1);
                printf("ElseBranch:\n");
                ast_print(node->data.if_stmt.else_branch, indent + 2);
            }
            break;

        case NODE_WHILE:
            printf("WhileStatement [Line %d]\n", node->line);
            print_indent(indent + 1);
            printf("Condition:\n");
            ast_print(node->data.while_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            ast_print(node->data.while_stmt.body, indent + 2);
            break;

        case NODE_BLOCK:
            printf("Block (statements: %zu) [Line %d]\n", node->data.block.stmt_count, node->line);
            for (size_t i = 0; i < node->data.block.stmt_count; i++) {
                ast_print(node->data.block.statements[i], indent + 1);
            }
            break;
    }
}

void ast_free(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            for (size_t i = 0; i < node->data.program.stmt_count; i++) {
                ast_free(node->data.program.statements[i]);
            }
            free(node->data.program.statements);
            break;

        case NODE_VAR_DECL:
            free(node->data.var_decl.var_name);
            ast_free(node->data.var_decl.init_expr);
            break;

        case NODE_ASSIGN:
            free(node->data.assign.var_name);
            ast_free(node->data.assign.expr);
            break;

        case NODE_BINARY_EXPR:
            ast_free(node->data.binary_expr.left);
            ast_free(node->data.binary_expr.right);
            break;

        case NODE_UNARY_EXPR:
            ast_free(node->data.unary_expr.operand);
            break;

        case NODE_INT_LITERAL:
        case NODE_FLOAT_LITERAL:
            break;

        case NODE_IDENTIFIER:
            free(node->data.identifier.name);
            break;

        case NODE_IF:
            ast_free(node->data.if_stmt.condition);
            ast_free(node->data.if_stmt.then_branch);
            ast_free(node->data.if_stmt.else_branch);
            break;

        case NODE_WHILE:
            ast_free(node->data.while_stmt.condition);
            ast_free(node->data.while_stmt.body);
            break;

        case NODE_BLOCK:
            for (size_t i = 0; i < node->data.block.stmt_count; i++) {
                ast_free(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;
    }

    free(node);
}
