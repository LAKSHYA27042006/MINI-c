#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

typedef enum {
    ERR_LEXICAL,
    ERR_SYNTAX,
    ERR_SEMANTIC,
    ERR_WARNING
} ErrorType;

typedef struct CompilerError {
    ErrorType type;
    int line;
    int column;
    char message[256];
    struct CompilerError *next;
} CompilerError;

typedef struct {
    CompilerError *head;
    CompilerError *tail;
    int count;
    int warning_count;
} ErrorReporter;

/* Initialize global or scoped error reporter */
void error_reporter_init(ErrorReporter *reporter);

/* Report an error with source location */
void error_report(ErrorReporter *reporter, ErrorType type, int line, int column, const char *fmt, ...);

/* Print all collected errors */
void error_reporter_print(const ErrorReporter *reporter);

/* Check if any fatal errors (non-warnings) have occurred */
bool error_reporter_has_errors(const ErrorReporter *reporter);

/* Free all errors */
void error_reporter_free(ErrorReporter *reporter);

#endif /* ERROR_H */
