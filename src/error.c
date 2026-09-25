#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void error_reporter_init(ErrorReporter *reporter) {
    if (!reporter) return;
    reporter->head = NULL;
    reporter->tail = NULL;
    reporter->count = 0;
    reporter->warning_count = 0;
}

void error_report(ErrorReporter *reporter, ErrorType type, int line, int column, const char *fmt, ...) {
    if (!reporter) return;

    CompilerError *err = (CompilerError *)malloc(sizeof(CompilerError));
    if (!err) {
        fprintf(stderr, "Fatal error: Out of memory while allocating error record.\n");
        exit(EXIT_FAILURE);
    }

    err->type = type;
    err->line = line;
    err->column = column;
    err->next = NULL;

    va_list args;
    va_start(args, fmt);
    vsnprintf(err->message, sizeof(err->message), fmt, args);
    va_end(args);

    if (type == ERR_WARNING) {
        reporter->warning_count++;
    } else {
        reporter->count++;
    }

    if (!reporter->head) {
        reporter->head = err;
        reporter->tail = err;
    } else {
        reporter->tail->next = err;
        reporter->tail = err;
    }
}

void error_reporter_print(const ErrorReporter *reporter) {
    if (!reporter || (!reporter->head)) return;

    CompilerError *curr = reporter->head;
    while (curr) {
        const char *type_str = "Error";
        switch (curr->type) {
            case ERR_LEXICAL:  type_str = "Lexical Error"; break;
            case ERR_SYNTAX:   type_str = "Syntax Error"; break;
            case ERR_SEMANTIC: type_str = "Semantic Error"; break;
            case ERR_WARNING:  type_str = "Warning"; break;
        }

        if (curr->line > 0 && curr->column > 0) {
            fprintf(stderr, "[%s] Line %d, Column %d: %s\n",
                    type_str, curr->line, curr->column, curr->message);
        } else if (curr->line > 0) {
            fprintf(stderr, "[%s] Line %d: %s\n",
                    type_str, curr->line, curr->message);
        } else {
            fprintf(stderr, "[%s]: %s\n",
                    type_str, curr->message);
        }

        curr = curr->next;
    }
}

bool error_reporter_has_errors(const ErrorReporter *reporter) {
    return reporter && (reporter->count > 0);
}

void error_reporter_free(ErrorReporter *reporter) {
    if (!reporter) return;
    CompilerError *curr = reporter->head;
    while (curr) {
        CompilerError *next = curr->next;
        free(curr);
        curr = next;
    }
    reporter->head = NULL;
    reporter->tail = NULL;
    reporter->count = 0;
    reporter->warning_count = 0;
}
