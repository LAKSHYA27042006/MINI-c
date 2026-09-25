#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    DATA_TYPE_INT,
    DATA_TYPE_FLOAT,
    DATA_TYPE_UNKNOWN
} DataType;

typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_IDENTIFIER,
    NODE_IF,
    NODE_WHILE,
    NODE_BLOCK
} ASTNodeType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NE
} BinaryOp;

typedef enum {
    UNARY_POS,
    UNARY_NEG
} UnaryOp;

typedef struct ASTNode ASTNode;

struct ASTNode {
    ASTNodeType type;
    int line;
    int column;
    DataType inferred_type;

    union {
        struct {
            ASTNode **statements;
            size_t stmt_count;
            size_t stmt_capacity;
        } program;

        struct {
            DataType var_type;
            char *var_name;
            ASTNode *init_expr;
        } var_decl;

        struct {
            char *var_name;
            ASTNode *expr;
        } assign;

        struct {
            BinaryOp op;
            ASTNode *left;
            ASTNode *right;
        } binary_expr;

        struct {
            UnaryOp op;
            ASTNode *operand;
        } unary_expr;

        struct {
            long long value;
        } int_literal;

        struct {
            double value;
        } float_literal;

        struct {
            char *name;
        } identifier;

        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch;
        } if_stmt;

        struct {
            ASTNode *condition;
            ASTNode *body;
        } while_stmt;

        struct {
            ASTNode **statements;
            size_t stmt_count;
            size_t stmt_capacity;
        } block;
    } data;
};

/* Constructors */
ASTNode *ast_create_program(void);
void ast_program_add_stmt(ASTNode *prog, ASTNode *stmt);

ASTNode *ast_create_var_decl(DataType type, const char *name, ASTNode *init_expr, int line, int col);
ASTNode *ast_create_assign(const char *name, ASTNode *expr, int line, int col);
ASTNode *ast_create_binary(BinaryOp op, ASTNode *left, ASTNode *right, int line, int col);
ASTNode *ast_create_unary(UnaryOp op, ASTNode *operand, int line, int col);
ASTNode *ast_create_int_literal(long long val, int line, int col);
ASTNode *ast_create_float_literal(double val, int line, int col);
ASTNode *ast_create_identifier(const char *name, int line, int col);
ASTNode *ast_create_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int line, int col);
ASTNode *ast_create_while(ASTNode *cond, ASTNode *body, int line, int col);
ASTNode *ast_create_block(int line, int col);
void ast_block_add_stmt(ASTNode *block, ASTNode *stmt);

/* Inspection and Cleanup */
const char *data_type_name(DataType type);
const char *binary_op_name(BinaryOp op);
const char *ast_node_type_name(ASTNodeType type);
void ast_print(const ASTNode *node, int indent);
void ast_free(ASTNode *node);

#endif /* AST_H */
