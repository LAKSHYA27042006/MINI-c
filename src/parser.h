#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include "error.h"

typedef struct {
    const TokenArray *tokens;
    size_t cursor;
    ErrorReporter *reporter;
    bool panic_mode;
} Parser;

/* Initialize parser */
void parser_init(Parser *parser, const TokenArray *tokens, ErrorReporter *reporter);

/* Parse full program and return AST root */
ASTNode *parser_parse_program(Parser *parser);

#endif /* PARSER_H */
