#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"
#include <stdbool.h>

#define MAX_SCOPE_DEPTH 32

typedef struct Symbol {
    char *name;
    DataType type;
    int scope_level;
    char scope_name[32];
    int line;
    int column;
    bool is_initialized;
    bool is_constant;
    union {
        long long int_val;
        double float_val;
    } const_val;
    struct Symbol *next; /* Linked list for hash bucket or scope chain */
} Symbol;

typedef struct Scope {
    int level;
    char name[32];
    Symbol *symbols;
    struct Scope *parent;
} Scope;

typedef struct {
    Scope *current_scope;
    Scope *all_scopes; /* Linked list of all created scopes for printing/record */
    int scope_counter;
} SymbolTable;

/* Symbol table lifecycle */
void symbol_table_init(SymbolTable *st);
void symbol_table_free(SymbolTable *st);

/* Scope operations */
void symbol_table_enter_scope(SymbolTable *st);
void symbol_table_exit_scope(SymbolTable *st);

/* Symbol insertion and lookups */
bool symbol_table_insert(SymbolTable *st, const char *name, DataType type, int line, int column);
Symbol *symbol_table_lookup_current(const SymbolTable *st, const char *name);
Symbol *symbol_table_lookup(const SymbolTable *st, const char *name);

/* Display */
void symbol_table_print(const SymbolTable *st);

#endif /* SYMBOL_TABLE_H */
