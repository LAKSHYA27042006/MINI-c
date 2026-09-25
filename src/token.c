#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_KW_INT:        return "KEYWORD_INT";
        case TOKEN_KW_FLOAT:      return "KEYWORD_FLOAT";
        case TOKEN_KW_IF:         return "KEYWORD_IF";
        case TOKEN_KW_ELSE:       return "KEYWORD_ELSE";
        case TOKEN_KW_WHILE:      return "KEYWORD_WHILE";
        case TOKEN_IDENTIFIER:    return "IDENTIFIER";
        case TOKEN_INT_LITERAL:   return "INTEGER_LITERAL";
        case TOKEN_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TOKEN_PLUS:          return "PLUS";
        case TOKEN_MINUS:         return "MINUS";
        case TOKEN_STAR:          return "MULTIPLY";
        case TOKEN_SLASH:         return "DIVIDE";
        case TOKEN_PERCENT:       return "MODULO";
        case TOKEN_ASSIGN:        return "ASSIGN";
        case TOKEN_LT:            return "LESS";
        case TOKEN_GT:            return "GREATER";
        case TOKEN_LE:            return "LESS_EQUAL";
        case TOKEN_GE:            return "GREATER_EQUAL";
        case TOKEN_EQ:            return "EQUAL_EQUAL";
        case TOKEN_NE:            return "NOT_EQUAL";
        case TOKEN_LPAREN:        return "LEFT_PAREN";
        case TOKEN_RPAREN:        return "RIGHT_PAREN";
        case TOKEN_LBRACE:        return "LEFT_BRACE";
        case TOKEN_RBRACE:        return "RIGHT_BRACE";
        case TOKEN_SEMICOLON:     return "SEMICOLON";
        case TOKEN_EOF:           return "EOF";
        case TOKEN_UNKNOWN:       return "UNKNOWN";
        default:                  return "INVALID";
    }
}

void token_init(Token *tok, TokenType type, const char *lexeme, int line, int column) {
    if (!tok) return;
    tok->type = type;
    tok->line = line;
    tok->column = column;
    tok->value.int_val = 0;
    tok->value.float_val = 0.0;

    if (lexeme) {
        size_t len = strlen(lexeme);
        tok->lexeme = (char *)malloc(len + 1);
        if (tok->lexeme) {
            memcpy(tok->lexeme, lexeme, len + 1);
        }
    } else {
        tok->lexeme = NULL;
    }
}

void token_free(Token *tok) {
    if (!tok) return;
    if (tok->lexeme) {
        free(tok->lexeme);
        tok->lexeme = NULL;
    }
}

void token_print(const Token *tok) {
    if (!tok) return;
    printf("[Line %3d, Col %3d] %-16s : '%s'",
           tok->line, tok->column, token_type_name(tok->type),
           tok->lexeme ? tok->lexeme : "");
    if (tok->type == TOKEN_INT_LITERAL) {
        printf(" (value: %ld)", (long)tok->value.int_val);
    } else if (tok->type == TOKEN_FLOAT_LITERAL) {
        printf(" (value: %g)", tok->value.float_val);
    }
    printf("\n");
}

void token_array_init(TokenArray *array) {
    if (!array) return;
    array->count = 0;
    array->capacity = 16;
    array->tokens = (Token *)malloc(sizeof(Token) * array->capacity);
    if (!array->tokens) {
        fprintf(stderr, "Fatal: Out of memory in token_array_init\n");
        exit(EXIT_FAILURE);
    }
}

void token_array_push(TokenArray *array, Token tok) {
    if (!array) return;
    if (array->count >= array->capacity) {
        array->capacity *= 2;
        Token *new_ptr = (Token *)realloc(array->tokens, sizeof(Token) * array->capacity);
        if (!new_ptr) {
            fprintf(stderr, "Fatal: Out of memory in token_array_push\n");
            exit(EXIT_FAILURE);
        }
        array->tokens = new_ptr;
    }
    array->tokens[array->count++] = tok;
}

void token_array_free(TokenArray *array) {
    if (!array) return;
    for (size_t i = 0; i < array->count; i++) {
        token_free(&array->tokens[i]);
    }
    free(array->tokens);
    array->tokens = NULL;
    array->count = 0;
    array->capacity = 0;
}

void token_array_print(const TokenArray *array) {
    if (!array) return;
    printf("========================================\n");
    printf("TOKEN STREAM (%zu tokens)\n", array->count);
    printf("========================================\n");
    for (size_t i = 0; i < array->count; i++) {
        token_print(&array->tokens[i]);
    }
    printf("========================================\n\n");
}
