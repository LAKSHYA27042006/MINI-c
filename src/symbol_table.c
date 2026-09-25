#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef struct ScopeNode {
    Scope *scope;
    struct ScopeNode *next;
} ScopeNode;

static ScopeNode *recorded_scopes = NULL;

static char *safe_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *res = (char *)malloc(len + 1);
    if (!res) {
        fprintf(stderr, "Fatal: Out of memory in safe_strdup\n");
        exit(EXIT_FAILURE);
    }
    memcpy(res, s, len + 1);
    return res;
}

static Scope *create_scope(int level, const char *name, Scope *parent) {
    Scope *sc = (Scope *)calloc(1, sizeof(Scope));
    if (!sc) {
        fprintf(stderr, "Fatal: Out of memory creating scope\n");
        exit(EXIT_FAILURE);
    }
    sc->level = level;
    strncpy(sc->name, name, sizeof(sc->name) - 1);
    sc->parent = parent;
    sc->symbols = NULL;

    /* Record for final report / print */
    ScopeNode *sn = (ScopeNode *)malloc(sizeof(ScopeNode));
    if (sn) {
        sn->scope = sc;
        sn->next = recorded_scopes;
        recorded_scopes = sn;
    }

    return sc;
}

void symbol_table_init(SymbolTable *st) {
    if (!st) return;
    recorded_scopes = NULL;
    st->scope_counter = 0;
    st->current_scope = create_scope(0, "global", NULL);
    st->all_scopes = st->current_scope;
}

void symbol_table_enter_scope(SymbolTable *st) {
    if (!st) return;
    st->scope_counter++;
    char name[32];
    snprintf(name, sizeof(name), "block-%d", st->scope_counter);
    int next_level = st->current_scope ? st->current_scope->level + 1 : 1;
    st->current_scope = create_scope(next_level, name, st->current_scope);
}

void symbol_table_exit_scope(SymbolTable *st) {
    if (!st || !st->current_scope) return;
    if (st->current_scope->parent) {
        st->current_scope = st->current_scope->parent;
    }
}

bool symbol_table_insert(SymbolTable *st, const char *name, DataType type, int line, int column) {
    if (!st || !st->current_scope || !name) return false;

    if (symbol_table_lookup_current(st, name) != NULL) {
        return false; /* Duplicate declaration in current scope */
    }

    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Fatal: Out of memory creating symbol\n");
        exit(EXIT_FAILURE);
    }

    sym->name = safe_strdup(name);
    sym->type = type;
    sym->scope_level = st->current_scope->level;
    strncpy(sym->scope_name, st->current_scope->name, sizeof(sym->scope_name) - 1);
    sym->line = line;
    sym->column = column;
    sym->is_initialized = false;
    sym->is_constant = false;
    sym->next = st->current_scope->symbols;
    st->current_scope->symbols = sym;

    return true;
}

Symbol *symbol_table_lookup_current(const SymbolTable *st, const char *name) {
    if (!st || !st->current_scope || !name) return NULL;

    Symbol *curr = st->current_scope->symbols;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

Symbol *symbol_table_lookup(const SymbolTable *st, const char *name) {
    if (!st || !name) return NULL;

    Scope *scope = st->current_scope;
    while (scope) {
        Symbol *curr = scope->symbols;
        while (curr) {
            if (strcmp(curr->name, name) == 0) {
                return curr;
            }
            curr = curr->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

void symbol_table_print(const SymbolTable *st) {
    (void)st;
    printf("================================================================================\n");
    printf("SYMBOL TABLE DIRECTORY\n");
    printf("================================================================================\n");
    printf("%-16s %-10s %-12s %-8s %-12s %-12s\n",
           "Identifier", "Type", "Scope", "Line", "Initialized", "Constant Val");
    printf("--------------------------------------------------------------------------------\n");

    /* Collect all recorded scopes in creation order */
    /* recorded_scopes is in reverse order, let's reverse to print in chronological order */
    size_t count = 0;
    ScopeNode *sn = recorded_scopes;
    while (sn) {
        count++;
        sn = sn->next;
    }

    if (count == 0) {
        printf("(No symbols registered)\n");
        printf("================================================================================\n\n");
        return;
    }

    Scope **list = (Scope **)malloc(sizeof(Scope *) * count);
    if (!list) {
        fprintf(stderr, "Fatal: Out of memory in symbol_table_print\n");
        exit(EXIT_FAILURE);
    }

    sn = recorded_scopes;
    for (size_t i = 0; i < count; i++) {
        list[count - 1 - i] = sn->scope;
        sn = sn->next;
    }

    bool any_symbols = false;
    for (size_t i = 0; i < count; i++) {
        Scope *sc = list[i];
        /* Reverse symbols in scope so they print in declaration order */
        size_t sym_count = 0;
        Symbol *sym = sc->symbols;
        while (sym) {
            sym_count++;
            sym = sym->next;
        }

        if (sym_count > 0) {
            any_symbols = true;
            Symbol **sym_arr = (Symbol **)malloc(sizeof(Symbol *) * sym_count);
            sym = sc->symbols;
            for (size_t j = 0; j < sym_count; j++) {
                sym_arr[sym_count - 1 - j] = sym;
                sym = sym->next;
            }

            for (size_t j = 0; j < sym_count; j++) {
                Symbol *s = sym_arr[j];
                char const_buf[32] = "-";
                if (s->is_constant) {
                    if (s->type == DATA_TYPE_INT) {
                        snprintf(const_buf, sizeof(const_buf), "%ld", (long)s->const_val.int_val);
                    } else if (s->type == DATA_TYPE_FLOAT) {
                        snprintf(const_buf, sizeof(const_buf), "%g", s->const_val.float_val);
                    }
                }

                printf("%-16s %-10s %-12s %-8d %-12s %-12s\n",
                       s->name,
                       data_type_name(s->type),
                       s->scope_name,
                       s->line,
                       s->is_initialized ? "Yes" : "No",
                       const_buf);
            }
            free(sym_arr);
        }
    }

    if (!any_symbols) {
        printf("(No variables declared)\n");
    }

    free(list);
    printf("================================================================================\n\n");
}

void symbol_table_free(SymbolTable *st) {
    if (!st) return;

    ScopeNode *sn = recorded_scopes;
    while (sn) {
        ScopeNode *next_sn = sn->next;
        Scope *sc = sn->scope;
        if (sc) {
            Symbol *sym = sc->symbols;
            while (sym) {
                Symbol *next_sym = sym->next;
                free(sym->name);
                free(sym);
                sym = next_sym;
            }
            free(sc);
        }
        free(sn);
        sn = next_sn;
    }
    recorded_scopes = NULL;
    st->current_scope = NULL;
    st->all_scopes = NULL;
    st->scope_counter = 0;
}
