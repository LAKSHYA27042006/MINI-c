#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

typedef enum {
    /* Keywords */
    TOKEN_KW_INT,
    TOKEN_KW_FLOAT,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_WHILE,

    /* Identifiers & Literals */
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,

    /* Arithmetic Operators */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    /* Assignment */
    TOKEN_ASSIGN,

    /* Relational Operators */
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_EQ,
    TOKEN_NE,

    /* Delimiters */
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,

    /* Special */
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int column;
    union {
        long long int_val;
        double float_val;
    } value;
} Token;

typedef struct {
    Token *tokens;
    size_t count;
    size_t capacity;
} TokenArray;

/* Token functions */
const char *token_type_name(TokenType type);
void token_init(Token *tok, TokenType type, const char *lexeme, int line, int column);
void token_free(Token *tok);
void token_print(const Token *tok);

/* TokenArray functions */
void token_array_init(TokenArray *array);
void token_array_push(TokenArray *array, Token tok);
void token_array_free(TokenArray *array);
void token_array_print(const TokenArray *array);

#endif /* TOKEN_H */
