#ifndef TAC_H
#define TAC_H

#include "ast.h"
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    TAC_ADD,
    TAC_SUB,
    TAC_MUL,
    TAC_DIV,
    TAC_MOD,
    TAC_ASSIGN,
    TAC_LT,
    TAC_GT,
    TAC_LE,
    TAC_GE,
    TAC_EQ,
    TAC_NE,
    TAC_IF_FALSE,
    TAC_IF_TRUE,
    TAC_GOTO,
    TAC_LABEL
} TACOp;

typedef enum {
    OPERAND_NONE,
    OPERAND_VAR,
    OPERAND_TEMP,
    OPERAND_CONST_INT,
    OPERAND_CONST_FLOAT,
    OPERAND_LABEL
} OperandType;

typedef struct {
    OperandType type;
    char name[64];
    DataType data_type;
    union {
        long long int_val;
        double float_val;
    } val;
} TACOperand;

typedef struct TACInstr {
    int id;
    TACOp op;
    TACOperand result;
    TACOperand arg1;
    TACOperand arg2;
    struct TACInstr *prev;
    struct TACInstr *next;
} TACInstr;

typedef struct {
    TACInstr *head;
    TACInstr *tail;
    size_t count;
    int temp_counter;
    int label_counter;
} TACProgram;

/* Operand helpers */
TACOperand operand_none(void);
TACOperand operand_var(const char *name, DataType type);
TACOperand operand_temp(const char *name, DataType type);
TACOperand operand_const_int(long long val);
TACOperand operand_const_float(double val);
TACOperand operand_label(const char *label_name);
bool operand_equals(const TACOperand *a, const TACOperand *b);
void operand_to_string(const TACOperand *op, char *buf, size_t size);

/* TAC Program functions */
void tac_program_init(TACProgram *prog);
void tac_program_free(TACProgram *prog);
TACInstr *tac_emit(TACProgram *prog, TACOp op, TACOperand res, TACOperand arg1, TACOperand arg2);
void tac_instr_remove(TACProgram *prog, TACInstr *instr);
TACInstr *tac_instr_clone(const TACInstr *src);
TACProgram *tac_program_clone(const TACProgram *src);

/* Generator helpers */
TACOperand tac_new_temp(TACProgram *prog, DataType type);
TACOperand tac_new_label(TACProgram *prog);

/* AST to TAC code generation */
void tac_generate(TACProgram *prog, const ASTNode *ast_root);

/* Printing */
void tac_instr_print(const TACInstr *instr);
void tac_program_print(const TACProgram *prog, const char *title);

#endif /* TAC_H */
