#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Token null_token = { TOKEN_EOF, NULL, 0, 0, {0} };

static const Token *peek(const Parser *parser) {
    if (!parser || !parser->tokens || parser->cursor >= parser->tokens->count) {
        return &null_token;
    }
    return &parser->tokens->tokens[parser->cursor];
}

static const Token *previous(const Parser *parser) {
    if (!parser || !parser->tokens || parser->cursor == 0) {
        return &null_token;
    }
    return &parser->tokens->tokens[parser->cursor - 1];
}

static bool is_at_end(const Parser *parser) {
    return peek(parser)->type == TOKEN_EOF;
}

static const Token *advance(Parser *parser) {
    if (!is_at_end(parser)) {
        parser->cursor++;
    }
    return previous(parser);
}

static bool check(const Parser *parser, TokenType type) {
    if (is_at_end(parser)) return false;
    return peek(parser)->type == type;
}

static bool match(Parser *parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return true;
    }
    return false;
}

static void parser_error(Parser *parser, const char *msg) {
    const Token *tok = peek(parser);
    if (!parser->panic_mode) {
        parser->panic_mode = true;
        if (parser->reporter) {
            error_report(parser->reporter, ERR_SYNTAX, tok->line, tok->column,
                         "%s (found '%s' of type %s)",
                         msg, tok->lexeme ? tok->lexeme : "EOF",
                         token_type_name(tok->type));
        }
    }
}


static void synchronize(Parser *parser) {
    parser->panic_mode = false;

    while (!is_at_end(parser)) {
        if (previous(parser)->type == TOKEN_SEMICOLON) return;

        switch (peek(parser)->type) {
            case TOKEN_KW_INT:
            case TOKEN_KW_FLOAT:
            case TOKEN_KW_IF:
            case TOKEN_KW_WHILE:
            case TOKEN_LBRACE:
            case TOKEN_RBRACE:
                return;
            default:
                break;
        }

        advance(parser);
    }
}

/* Forward declarations */
static ASTNode *statement(Parser *parser);
static ASTNode *declaration(Parser *parser);
static ASTNode *assignment(Parser *parser);
static ASTNode *if_statement(Parser *parser);
static ASTNode *while_statement(Parser *parser);
static ASTNode *block(Parser *parser);
static ASTNode *condition(Parser *parser);
static ASTNode *expression(Parser *parser);
static ASTNode *term(Parser *parser);
static ASTNode *factor(Parser *parser);

void parser_init(Parser *parser, const TokenArray *tokens, ErrorReporter *reporter) {
    if (!parser) return;
    parser->tokens = tokens;
    parser->cursor = 0;
    parser->reporter = reporter;
    parser->panic_mode = false;
}

ASTNode *parser_parse_program(Parser *parser) {
    ASTNode *prog = ast_create_program();

    while (!is_at_end(parser)) {
        ASTNode *stmt = statement(parser);
        if (stmt) {
            ast_program_add_stmt(prog, stmt);
        } else if (parser->panic_mode) {
            synchronize(parser);
        }
    }

    return prog;
}

static ASTNode *statement(Parser *parser) {
    if (check(parser, TOKEN_KW_INT) || check(parser, TOKEN_KW_FLOAT)) {
        return declaration(parser);
    }
    if (check(parser, TOKEN_KW_IF)) {
        return if_statement(parser);
    }
    if (check(parser, TOKEN_KW_WHILE)) {
        return while_statement(parser);
    }
    if (check(parser, TOKEN_LBRACE)) {
        return block(parser);
    }
    if (check(parser, TOKEN_IDENTIFIER)) {
        return assignment(parser);
    }

    parser_error(parser, "Expected statement (declaration, assignment, if, while, or block)");
    return NULL;
}

static ASTNode *declaration(Parser *parser) {
    const Token *type_tok = advance(parser);
    DataType dtype = (type_tok->type == TOKEN_KW_INT) ? DATA_TYPE_INT : DATA_TYPE_FLOAT;

    if (!check(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, "Expected identifier after type keyword");
        return NULL;
    }
    const Token *id_tok = advance(parser);
    const char *name = id_tok->lexeme;
    int line = id_tok->line;
    int col = id_tok->column;

    ASTNode *init_expr = NULL;
    if (match(parser, TOKEN_ASSIGN)) {
        init_expr = expression(parser);
        if (!init_expr) return NULL;
    }

    if (!match(parser, TOKEN_SEMICOLON)) {
        parser_error(parser, "Expected ';' after variable declaration");
        return NULL;
    }

    return ast_create_var_decl(dtype, name, init_expr, line, col);
}

static ASTNode *assignment(Parser *parser) {
    const Token *id_tok = advance(parser);
    const char *name = id_tok->lexeme;
    int line = id_tok->line;
    int col = id_tok->column;

    if (!match(parser, TOKEN_ASSIGN)) {
        parser_error(parser, "Expected '=' in assignment");
        return NULL;
    }

    ASTNode *expr = expression(parser);
    if (!expr) return NULL;

    if (!match(parser, TOKEN_SEMICOLON)) {
        parser_error(parser, "Expected ';' after assignment");
        ast_free(expr);
        return NULL;
    }

    return ast_create_assign(name, expr, line, col);
}

static ASTNode *if_statement(Parser *parser) {
    const Token *if_tok = advance(parser); // consume 'if'
    int line = if_tok->line;
    int col = if_tok->column;

    if (!match(parser, TOKEN_LPAREN)) {
        parser_error(parser, "Expected '(' after 'if'");
        return NULL;
    }

    ASTNode *cond = condition(parser);
    if (!cond) return NULL;

    if (!match(parser, TOKEN_RPAREN)) {
        parser_error(parser, "Expected ')' after if condition");
        ast_free(cond);
        return NULL;
    }

    ASTNode *then_b = statement(parser);
    if (!then_b) {
        ast_free(cond);
        return NULL;
    }

    ASTNode *else_b = NULL;
    if (match(parser, TOKEN_KW_ELSE)) {
        else_b = statement(parser);
        if (!else_b) {
            ast_free(cond);
            ast_free(then_b);
            return NULL;
        }
    }

    return ast_create_if(cond, then_b, else_b, line, col);
}

static ASTNode *while_statement(Parser *parser) {
    const Token *while_tok = advance(parser); // consume 'while'
    int line = while_tok->line;
    int col = while_tok->column;

    if (!match(parser, TOKEN_LPAREN)) {
        parser_error(parser, "Expected '(' after 'while'");
        return NULL;
    }

    ASTNode *cond = condition(parser);
    if (!cond) return NULL;

    if (!match(parser, TOKEN_RPAREN)) {
        parser_error(parser, "Expected ')' after while condition");
        ast_free(cond);
        return NULL;
    }

    ASTNode *body = statement(parser);
    if (!body) {
        ast_free(cond);
        return NULL;
    }

    return ast_create_while(cond, body, line, col);
}

static ASTNode *block(Parser *parser) {
    const Token *brace = advance(parser); // consume '{'
    int line = brace->line;
    int col = brace->column;

    ASTNode *blk = ast_create_block(line, col);

    while (!check(parser, TOKEN_RBRACE) && !is_at_end(parser)) {
        ASTNode *stmt = statement(parser);
        if (stmt) {
            ast_block_add_stmt(blk, stmt);
        } else if (parser->panic_mode) {
            synchronize(parser);
        }
    }

    if (!match(parser, TOKEN_RBRACE)) {
        parser_error(parser, "Expected '}' at end of block");
        ast_free(blk);
        return NULL;
    }

    return blk;
}

static ASTNode *condition(Parser *parser) {
    ASTNode *left = expression(parser);
    if (!left) return NULL;

    BinaryOp op;
    bool has_rel_op = true;

    if (match(parser, TOKEN_LT))       op = OP_LT;
    else if (match(parser, TOKEN_GT))  op = OP_GT;
    else if (match(parser, TOKEN_LE))  op = OP_LE;
    else if (match(parser, TOKEN_GE))  op = OP_GE;
    else if (match(parser, TOKEN_EQ))  op = OP_EQ;
    else if (match(parser, TOKEN_NE))  op = OP_NE;
    else has_rel_op = false;

    if (has_rel_op) {
        int line = previous(parser)->line;
        int col = previous(parser)->column;
        ASTNode *right = expression(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        return ast_create_binary(op, left, right, line, col);
    }

    return left;
}

static ASTNode *expression(Parser *parser) {
    ASTNode *node = term(parser);
    if (!node) return NULL;

    while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MINUS)) {
        const Token *op_tok = advance(parser);
        BinaryOp op = (op_tok->type == TOKEN_PLUS) ? OP_ADD : OP_SUB;
        int line = op_tok->line;
        int col = op_tok->column;

        ASTNode *right = term(parser);
        if (!right) {
            ast_free(node);
            return NULL;
        }

        node = ast_create_binary(op, node, right, line, col);
    }

    return node;
}

static ASTNode *term(Parser *parser) {
    ASTNode *node = factor(parser);
    if (!node) return NULL;

    while (check(parser, TOKEN_STAR) || check(parser, TOKEN_SLASH) || check(parser, TOKEN_PERCENT)) {
        const Token *op_tok = advance(parser);
        BinaryOp op;
        if (op_tok->type == TOKEN_STAR) op = OP_MUL;
        else if (op_tok->type == TOKEN_SLASH) op = OP_DIV;
        else op = OP_MOD;

        int line = op_tok->line;
        int col = op_tok->column;

        ASTNode *right = factor(parser);
        if (!right) {
            ast_free(node);
            return NULL;
        }

        node = ast_create_binary(op, node, right, line, col);
    }

    return node;
}

static ASTNode *factor(Parser *parser) {
    if (match(parser, TOKEN_INT_LITERAL)) {
        const Token *tok = previous(parser);
        return ast_create_int_literal(tok->value.int_val, tok->line, tok->column);
    }

    if (match(parser, TOKEN_FLOAT_LITERAL)) {
        const Token *tok = previous(parser);
        return ast_create_float_literal(tok->value.float_val, tok->line, tok->column);
    }

    if (match(parser, TOKEN_IDENTIFIER)) {
        const Token *tok = previous(parser);
        return ast_create_identifier(tok->lexeme, tok->line, tok->column);
    }

    if (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        const Token *tok = previous(parser);
        UnaryOp uop = (tok->type == TOKEN_MINUS) ? UNARY_NEG : UNARY_POS;
        ASTNode *opnd = factor(parser);
        if (!opnd) return NULL;
        return ast_create_unary(uop, opnd, tok->line, tok->column);
    }

    if (match(parser, TOKEN_LPAREN)) {
        ASTNode *expr = expression(parser);
        if (!expr) return NULL;

        if (!match(parser, TOKEN_RPAREN)) {
            parser_error(parser, "Expected ')' to close parenthesized expression");
            ast_free(expr);
            return NULL;
        }
        return expr;
    }

    parser_error(parser, "Expected expression factor (number, identifier, or '(')");
    return NULL;
}
