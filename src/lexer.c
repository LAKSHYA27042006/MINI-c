#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void lexer_init(Lexer *lexer, const char *source, ErrorReporter *reporter) {
    if (!lexer) return;
    lexer->source = source ? source : "";
    lexer->length = strlen(lexer->source);
    lexer->cursor = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->reporter = reporter;
}

static char peek_char(const Lexer *lexer) {
    if (lexer->cursor >= lexer->length) return '\0';
    return lexer->source[lexer->cursor];
}

static char peek_next_char(const Lexer *lexer) {
    if (lexer->cursor + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->cursor + 1];
}

static char advance_char(Lexer *lexer) {
    if (lexer->cursor >= lexer->length) return '\0';
    char c = lexer->source[lexer->cursor++];
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else if (c == '\t') {
        lexer->column += 4;
    } else {
        lexer->column++;
    }
    return c;
}

static void skip_whitespace_and_comments(Lexer *lexer) {
    while (lexer->cursor < lexer->length) {
        char c = peek_char(lexer);

        /* Whitespace */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance_char(lexer);
            continue;
        }

        /* Comments */
        if (c == '/' && peek_next_char(lexer) == '/') {
            /* Single-line comment */
            advance_char(lexer); // consume '/'
            advance_char(lexer); // consume '/'
            while (peek_char(lexer) != '\0' && peek_char(lexer) != '\n') {
                advance_char(lexer);
            }
            continue;
        }

        if (c == '/' && peek_next_char(lexer) == '*') {
            /* Multi-line comment */
            int start_line = lexer->line;
            int start_col = lexer->column;
            advance_char(lexer); // consume '/'
            advance_char(lexer); // consume '*'

            bool closed = false;
            while (peek_char(lexer) != '\0') {
                if (peek_char(lexer) == '*' && peek_next_char(lexer) == '/') {
                    advance_char(lexer); // consume '*'
                    advance_char(lexer); // consume '/'
                    closed = true;
                    break;
                }
                advance_char(lexer);
            }

            if (!closed) {
                if (lexer->reporter) {
                    error_report(lexer->reporter, ERR_LEXICAL, start_line, start_col,
                                 "Unterminated multi-line comment starting here");
                }
            }
            continue;
        }

        /* Not whitespace or comment */
        break;
    }
}

TokenArray lexer_tokenize(Lexer *lexer) {
    TokenArray array;
    token_array_init(&array);

    if (!lexer) return array;

    while (true) {
        skip_whitespace_and_comments(lexer);

        int token_line = lexer->line;
        int token_col = lexer->column;
        char c = peek_char(lexer);

        if (c == '\0') {
            Token eof_tok;
            token_init(&eof_tok, TOKEN_EOF, "EOF", token_line, token_col);
            token_array_push(&array, eof_tok);
            break;
        }

        /* Identifiers and Keywords */
        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = lexer->cursor;
            while (isalnum((unsigned char)peek_char(lexer)) || peek_char(lexer) == '_') {
                advance_char(lexer);
            }
            size_t len = lexer->cursor - start;
            char *buf = (char *)malloc(len + 1);
            if (!buf) {
                fprintf(stderr, "Fatal: Out of memory in lexer\n");
                exit(EXIT_FAILURE);
            }
            memcpy(buf, lexer->source + start, len);
            buf[len] = '\0';

            TokenType type = TOKEN_IDENTIFIER;
            if (strcmp(buf, "int") == 0)        type = TOKEN_KW_INT;
            else if (strcmp(buf, "float") == 0) type = TOKEN_KW_FLOAT;
            else if (strcmp(buf, "if") == 0)    type = TOKEN_KW_IF;
            else if (strcmp(buf, "else") == 0)  type = TOKEN_KW_ELSE;
            else if (strcmp(buf, "while") == 0) type = TOKEN_KW_WHILE;

            Token tok;
            token_init(&tok, type, buf, token_line, token_col);
            token_array_push(&array, tok);
            free(buf);
            continue;
        }

        /* Numeric Literals */
        if (isdigit((unsigned char)c)) {
            size_t start = lexer->cursor;
            bool is_float = false;

            while (isdigit((unsigned char)peek_char(lexer))) {
                advance_char(lexer);
            }

            if (peek_char(lexer) == '.' && isdigit((unsigned char)peek_next_char(lexer))) {
                is_float = true;
                advance_char(lexer); // consume '.'
                while (isdigit((unsigned char)peek_char(lexer))) {
                    advance_char(lexer);
                }
            }

            size_t len = lexer->cursor - start;
            char *buf = (char *)malloc(len + 1);
            if (!buf) {
                fprintf(stderr, "Fatal: Out of memory in lexer\n");
                exit(EXIT_FAILURE);
            }
            memcpy(buf, lexer->source + start, len);
            buf[len] = '\0';

            Token tok;
            if (is_float) {
                token_init(&tok, TOKEN_FLOAT_LITERAL, buf, token_line, token_col);
                tok.value.float_val = atof(buf);
            } else {
                token_init(&tok, TOKEN_INT_LITERAL, buf, token_line, token_col);
                tok.value.int_val = atoll(buf);
            }
            token_array_push(&array, tok);
            free(buf);
            continue;
        }

        /* Operators and Delimiters */
        advance_char(lexer); // consume first character

        Token tok;
        switch (c) {
            case '+':
                token_init(&tok, TOKEN_PLUS, "+", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case '-':
                token_init(&tok, TOKEN_MINUS, "-", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case '*':
                token_init(&tok, TOKEN_STAR, "*", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case '/':
                token_init(&tok, TOKEN_SLASH, "/", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case '%':
                token_init(&tok, TOKEN_PERCENT, "%", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case '=':
                if (peek_char(lexer) == '=') {
                    advance_char(lexer);
                    token_init(&tok, TOKEN_EQ, "==", token_line, token_col);
                } else {
                    token_init(&tok, TOKEN_ASSIGN, "=", token_line, token_col);
                }
                token_array_push(&array, tok);
                break;
            case '<':
                if (peek_char(lexer) == '=') {
                    advance_char(lexer);
                    token_init(&tok, TOKEN_LE, "<=", token_line, token_col);
                } else {
                    token_init(&tok, TOKEN_LT, "<", token_line, token_col);
                }
                token_array_push(&array, tok);
                break;
            case '>':
                if (peek_char(lexer) == '=') {
                    advance_char(lexer);
                    token_init(&tok, TOKEN_GE, ">=", token_line, token_col);
                } else {
                    token_init(&tok, TOKEN_GT, ">", token_line, token_col);
                }
                token_array_push(&array, tok);
                break;
            case '!':
                if (peek_char(lexer) == '=') {
                    advance_char(lexer);
                    token_init(&tok, TOKEN_NE, "!=", token_line, token_col);
                    token_array_push(&array, tok);
                } else {
                    if (lexer->reporter) {
                        error_report(lexer->reporter, ERR_LEXICAL, token_line, token_col,
                                     "Unexpected character '!' (did you mean '!='?)");
                    }
                }
                break;
            case '(':
                token_init(&tok, TOKEN_LPAREN, "(", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case ')':
                token_init(&tok, TOKEN_RPAREN, ")", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case '{':
                token_init(&tok, TOKEN_LBRACE, "{", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case '}':
                token_init(&tok, TOKEN_RBRACE, "}", token_line, token_col);
                token_array_push(&array, tok);
                break;
            case ';':
                token_init(&tok, TOKEN_SEMICOLON, ";", token_line, token_col);
                token_array_push(&array, tok);
                break;
            default:
                if (lexer->reporter) {
                    error_report(lexer->reporter, ERR_LEXICAL, token_line, token_col,
                                 "Unexpected character '%c' (ASCII %d)", c, (int)c);
                }
                break;
        }
    }

    return array;
}
