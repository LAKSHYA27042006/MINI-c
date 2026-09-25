#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include "token.h"
#include "error.h"

typedef struct {
    const char *source;
    size_t length;
    size_t cursor;
    int line;
    int column;
    ErrorReporter *reporter;
} Lexer;

/* Initialize lexer with source string and error reporter */
void lexer_init(Lexer *lexer, const char *source, ErrorReporter *reporter);

/* Tokenize the entire source into a TokenArray */
TokenArray lexer_tokenize(Lexer *lexer);

#endif /* LEXER_H */
