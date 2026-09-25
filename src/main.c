#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol_table.h"
#include "semantic.h"
#include "tac.h"
#include "optimizer.h"

typedef struct {
    bool show_tokens;
    bool show_ast;
    bool show_symtab;
    bool show_tac;
    bool show_optimized;
    bool show_report;
} CliOptions;

static char *read_file_to_string(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file '%s'\n", path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (length < 0) {
        fclose(fp);
        return NULL;
    }

    char *buffer = (char *)malloc(length + 1);
    if (!buffer) {
        fclose(fp);
        fprintf(stderr, "Fatal: Out of memory reading file '%s'\n", path);
        return NULL;
    }

    size_t read_bytes = fread(buffer, 1, length, fp);
    buffer[read_bytes] = '\0';
    fclose(fp);
    return buffer;
}

static bool compile_source(const char *source, const char *source_name, CliOptions opts) {
    ErrorReporter reporter;
    error_reporter_init(&reporter);

    printf("========================================\n");
    printf("Compiling: %s\n", source_name);
    printf("========================================\n");

    /* 1. Lexical Analysis */
    Lexer lexer;
    lexer_init(&lexer, source, &reporter);
    TokenArray tokens = lexer_tokenize(&lexer);

    if (opts.show_tokens) {
        token_array_print(&tokens);
    }

    if (error_reporter_has_errors(&reporter)) {
        printf("\n>>> Compilation halted due to Lexical Errors:\n");
        error_reporter_print(&reporter);
        token_array_free(&tokens);
        error_reporter_free(&reporter);
        return false;
    }

    /* 2. Syntax Analysis */
    Parser parser;
    parser_init(&parser, &tokens, &reporter);
    ASTNode *ast_root = parser_parse_program(&parser);

    if (opts.show_ast && ast_root) {
        printf("========================================\n");
        printf("ABSTRACT SYNTAX TREE (AST)\n");
        printf("========================================\n");
        ast_print(ast_root, 0);
        printf("========================================\n\n");
    }

    if (error_reporter_has_errors(&reporter)) {
        printf("\n>>> Compilation halted due to Syntax Errors:\n");
        error_reporter_print(&reporter);
        ast_free(ast_root);
        token_array_free(&tokens);
        error_reporter_free(&reporter);
        return false;
    }

    /* 3. Semantic Analysis & Symbol Table */
    SymbolTable symtab;
    symbol_table_init(&symtab);

    SemanticAnalyzer sem_analyzer;
    semantic_analyzer_init(&sem_analyzer, &symtab, &reporter);
    semantic_analyze(&sem_analyzer, ast_root);

    if (opts.show_symtab) {
        symbol_table_print(&symtab);
    }

    if (error_reporter_has_errors(&reporter)) {
        printf("\n>>> Compilation halted due to Semantic Errors:\n");
        error_reporter_print(&reporter);
        symbol_table_free(&symtab);
        ast_free(ast_root);
        token_array_free(&tokens);
        error_reporter_free(&reporter);
        return false;
    }

    /* Print any non-fatal semantic warnings */
    if (reporter.warning_count > 0) {
        printf(">>> Compilation Warnings:\n");
        error_reporter_print(&reporter);
        printf("\n");
    }

    /* 4. Three-Address Code Generation */
    TACProgram raw_tac;
    tac_program_init(&raw_tac);
    tac_generate(&raw_tac, ast_root);

    if (opts.show_tac) {
        tac_program_print(&raw_tac, "RAW THREE-ADDRESS CODE (UNOPTIMIZED)");
    }

    /* 5. Peephole Optimization */
    OptimizationStats opt_stats;
    TACProgram *opt_tac = optimize_tac(&raw_tac, &opt_stats);

    if (opts.show_optimized && opt_tac) {
        tac_program_print(opt_tac, "OPTIMIZED THREE-ADDRESS CODE");
    }

    /* 6. Optimization Report */
    if (opts.show_report) {
        optimization_report_print(&opt_stats);
    }

    printf(">>> Compilation successful!\n\n");

    /* Cleanup */
    if (opt_tac) {
        tac_program_free(opt_tac);
        free(opt_tac);
    }
    tac_program_free(&raw_tac);
    symbol_table_free(&symtab);
    ast_free(ast_root);
    token_array_free(&tokens);
    error_reporter_free(&reporter);

    return true;
}

static void print_interactive_menu(void) {
    printf("========================================\n");
    printf("           MiniC Compiler               \n");
    printf("========================================\n");
    printf("1. Enter source code interactively\n");
    printf("2. Compile source file\n");
    printf("3. Run demo test case\n");
    printf("4. Exit\n");
    printf("----------------------------------------\n");
    printf("Select an option (1-4): ");
}

static void run_interactive_mode(void) {
    char choice_buf[64];

    while (1) {
        print_interactive_menu();
        if (!fgets(choice_buf, sizeof(choice_buf), stdin)) {
            break;
        }

        int choice = atoi(choice_buf);

        CliOptions all_opts;
        all_opts.show_tokens = true;
        all_opts.show_ast = true;
        all_opts.show_symtab = true;
        all_opts.show_tac = true;
        all_opts.show_optimized = true;
        all_opts.show_report = true;

        if (choice == 1) {
            printf("\nEnter MiniC code. Type 'END' on a separate line when finished:\n");
            char buffer[4096] = {0};
            char line[256];
            while (fgets(line, sizeof(line), stdin)) {
                if (strncmp(line, "END", 3) == 0 && (line[3] == '\n' || line[3] == '\r' || line[3] == '\0')) {
                    break;
                }
                strncat(buffer, line, sizeof(buffer) - strlen(buffer) - 1);
            }
            compile_source(buffer, "interactive_input.mc", all_opts);
        } else if (choice == 2) {
            printf("\nEnter file path to compile: ");
            char path[256];
            if (fgets(path, sizeof(path), stdin)) {
                size_t len = strlen(path);
                while (len > 0 && (path[len - 1] == '\n' || path[len - 1] == '\r' || path[len - 1] == ' ')) {
                    path[--len] = '\0';
                }
                char *src = read_file_to_string(path);
                if (src) {
                    compile_source(src, path, all_opts);
                    free(src);
                }
            }
        } else if (choice == 3) {
            const char *demo_src =
                "int x = 3 + 4;\n"
                "int y = x + 0;\n"
                "\n"
                "while (y < 20) {\n"
                "    y = y + 1;\n"
                "}\n";
            printf("\nRunning Primary Academic Demo:\n");
            printf("Source Code:\n%s\n", demo_src);
            compile_source(demo_src, "academic_demo.mc", all_opts);
        } else if (choice == 4) {
            printf("Exiting MiniC Compiler. Goodbye!\n");
            break;
        } else {
            printf("Invalid selection. Please choose 1, 2, 3, or 4.\n\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        run_interactive_mode();
        return 0;
    }

    const char *file_path = NULL;
    CliOptions opts = { false, false, false, true, true, true }; // default shows TAC, opt TAC, and report

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) {
            opts.show_tokens = true;
        } else if (strcmp(argv[i], "--ast") == 0) {
            opts.show_ast = true;
        } else if (strcmp(argv[i], "--symtab") == 0) {
            opts.show_symtab = true;
        } else if (strcmp(argv[i], "--tac") == 0) {
            opts.show_tac = true;
        } else if (strcmp(argv[i], "--optimized") == 0) {
            opts.show_optimized = true;
        } else if (strcmp(argv[i], "--report") == 0) {
            opts.show_report = true;
        } else if (strcmp(argv[i], "--all") == 0) {
            opts.show_tokens = true;
            opts.show_ast = true;
            opts.show_symtab = true;
            opts.show_tac = true;
            opts.show_optimized = true;
            opts.show_report = true;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown flag: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s <source.mc> [--tokens] [--ast] [--symtab] [--tac] [--optimized] [--report] [--all]\n", argv[0]);
            return 1;
        } else {
            file_path = argv[i];
        }
    }

    if (!file_path) {
        fprintf(stderr, "Error: No input source file specified.\n");
        fprintf(stderr, "Usage: %s <source.mc> [flags]\n", argv[0]);
        return 1;
    }

    char *source = read_file_to_string(file_path);
    if (!source) {
        return 1;
    }

    bool success = compile_source(source, file_path, opts);
    free(source);

    return success ? 0 : 1;
}
