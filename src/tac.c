#include "tac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

TACOperand operand_none(void) {
    TACOperand op;
    memset(&op, 0, sizeof(op));
    op.type = OPERAND_NONE;
    op.data_type = DATA_TYPE_UNKNOWN;
    return op;
}

TACOperand operand_var(const char *name, DataType type) {
    TACOperand op;
    memset(&op, 0, sizeof(op));
    op.type = OPERAND_VAR;
    op.data_type = type;
    if (name) {
        strncpy(op.name, name, sizeof(op.name) - 1);
    }
    return op;
}

TACOperand operand_temp(const char *name, DataType type) {
    TACOperand op;
    memset(&op, 0, sizeof(op));
    op.type = OPERAND_TEMP;
    op.data_type = type;
    if (name) {
        strncpy(op.name, name, sizeof(op.name) - 1);
    }
    return op;
}

TACOperand operand_const_int(long long val) {
    TACOperand op;
    memset(&op, 0, sizeof(op));
    op.type = OPERAND_CONST_INT;
    op.data_type = DATA_TYPE_INT;
    op.val.int_val = val;
    snprintf(op.name, sizeof(op.name), "%ld", (long)val);
    return op;
}

TACOperand operand_const_float(double val) {
    TACOperand op;
    memset(&op, 0, sizeof(op));
    op.type = OPERAND_CONST_FLOAT;
    op.data_type = DATA_TYPE_FLOAT;
    op.val.float_val = val;
    snprintf(op.name, sizeof(op.name), "%g", val);
    return op;
}

TACOperand operand_label(const char *label_name) {
    TACOperand op;
    memset(&op, 0, sizeof(op));
    op.type = OPERAND_LABEL;
    op.data_type = DATA_TYPE_UNKNOWN;
    if (label_name) {
        strncpy(op.name, label_name, sizeof(op.name) - 1);
    }
    return op;
}

bool operand_equals(const TACOperand *a, const TACOperand *b) {
    if (!a || !b) return false;
    if (a->type != b->type) return false;

    switch (a->type) {
        case OPERAND_NONE:
            return true;
        case OPERAND_VAR:
        case OPERAND_TEMP:
        case OPERAND_LABEL:
            return strcmp(a->name, b->name) == 0;
        case OPERAND_CONST_INT:
            return a->val.int_val == b->val.int_val;
        case OPERAND_CONST_FLOAT:
            return a->val.float_val == b->val.float_val;
    }
    return false;
}

void operand_to_string(const TACOperand *op, char *buf, size_t size) {
    if (!op || !buf || size == 0) return;

    switch (op->type) {
        case OPERAND_NONE:
            buf[0] = '\0';
            break;
        case OPERAND_VAR:
        case OPERAND_TEMP:
        case OPERAND_LABEL:
            strncpy(buf, op->name, size - 1);
            buf[size - 1] = '\0';
            break;
        case OPERAND_CONST_INT:
            snprintf(buf, size, "%ld", (long)op->val.int_val);
            break;
        case OPERAND_CONST_FLOAT:
            snprintf(buf, size, "%g", op->val.float_val);
            break;
    }
}

void tac_program_init(TACProgram *prog) {
    if (!prog) return;
    prog->head = NULL;
    prog->tail = NULL;
    prog->count = 0;
    prog->temp_counter = 0;
    prog->label_counter = 0;
}

void tac_program_free(TACProgram *prog) {
    if (!prog) return;
    TACInstr *curr = prog->head;
    while (curr) {
        TACInstr *next = curr->next;
        free(curr);
        curr = next;
    }
    prog->head = NULL;
    prog->tail = NULL;
    prog->count = 0;
}

TACInstr *tac_emit(TACProgram *prog, TACOp op, TACOperand res, TACOperand arg1, TACOperand arg2) {
    if (!prog) return NULL;

    TACInstr *instr = (TACInstr *)calloc(1, sizeof(TACInstr));
    if (!instr) {
        fprintf(stderr, "Fatal: Out of memory in tac_emit\n");
        exit(EXIT_FAILURE);
    }

    instr->id = (int)prog->count + 1;
    instr->op = op;
    instr->result = res;
    instr->arg1 = arg1;
    instr->arg2 = arg2;
    instr->prev = prog->tail;
    instr->next = NULL;

    if (prog->tail) {
        prog->tail->next = instr;
        prog->tail = instr;
    } else {
        prog->head = instr;
        prog->tail = instr;
    }

    prog->count++;
    return instr;
}

void tac_instr_remove(TACProgram *prog, TACInstr *instr) {
    if (!prog || !instr) return;

    if (instr->prev) {
        instr->prev->next = instr->next;
    } else {
        prog->head = instr->next;
    }

    if (instr->next) {
        instr->next->prev = instr->prev;
    } else {
        prog->tail = instr->prev;
    }

    prog->count--;
    free(instr);
}

TACInstr *tac_instr_clone(const TACInstr *src) {
    if (!src) return NULL;
    TACInstr *copy = (TACInstr *)calloc(1, sizeof(TACInstr));
    if (!copy) {
        fprintf(stderr, "Fatal: Out of memory in tac_instr_clone\n");
        exit(EXIT_FAILURE);
    }
    *copy = *src;
    copy->prev = NULL;
    copy->next = NULL;
    return copy;
}

TACProgram *tac_program_clone(const TACProgram *src) {
    if (!src) return NULL;
    TACProgram *copy = (TACProgram *)malloc(sizeof(TACProgram));
    if (!copy) {
        fprintf(stderr, "Fatal: Out of memory in tac_program_clone\n");
        exit(EXIT_FAILURE);
    }
    tac_program_init(copy);
    copy->temp_counter = src->temp_counter;
    copy->label_counter = src->label_counter;

    TACInstr *curr = src->head;
    while (curr) {
        tac_emit(copy, curr->op, curr->result, curr->arg1, curr->arg2);
        curr = curr->next;
    }

    return copy;
}

TACOperand tac_new_temp(TACProgram *prog, DataType type) {
    prog->temp_counter++;
    char name[32];
    snprintf(name, sizeof(name), "t%d", prog->temp_counter);
    return operand_temp(name, type);
}

TACOperand tac_new_label(TACProgram *prog) {
    prog->label_counter++;
    char name[32];
    snprintf(name, sizeof(name), "L%d", prog->label_counter);
    return operand_label(name);
}

/* AST -> TAC Code Generation */
static TACOperand gen_expr(TACProgram *prog, const ASTNode *node);
static void gen_stmt(TACProgram *prog, const ASTNode *node);

static TACOperand gen_expr(TACProgram *prog, const ASTNode *node) {
    if (!node) return operand_none();

    switch (node->type) {
        case NODE_INT_LITERAL:
            return operand_const_int(node->data.int_literal.value);

        case NODE_FLOAT_LITERAL:
            return operand_const_float(node->data.float_literal.value);

        case NODE_IDENTIFIER:
            return operand_var(node->data.identifier.name, node->inferred_type);

        case NODE_UNARY_EXPR: {
            TACOperand opnd = gen_expr(prog, node->data.unary_expr.operand);
            if (node->data.unary_expr.op == UNARY_NEG) {
                TACOperand zero = (node->inferred_type == DATA_TYPE_FLOAT)
                                  ? operand_const_float(0.0)
                                  : operand_const_int(0);
                TACOperand temp = tac_new_temp(prog, node->inferred_type);
                tac_emit(prog, TAC_SUB, temp, zero, opnd);
                return temp;
            }
            return opnd;
        }

        case NODE_BINARY_EXPR: {
            TACOperand left = gen_expr(prog, node->data.binary_expr.left);
            TACOperand right = gen_expr(prog, node->data.binary_expr.right);

            TACOp tac_op;
            switch (node->data.binary_expr.op) {
                case OP_ADD: tac_op = TAC_ADD; break;
                case OP_SUB: tac_op = TAC_SUB; break;
                case OP_MUL: tac_op = TAC_MUL; break;
                case OP_DIV: tac_op = TAC_DIV; break;
                case OP_MOD: tac_op = TAC_MOD; break;
                case OP_LT:  tac_op = TAC_LT;  break;
                case OP_GT:  tac_op = TAC_GT;  break;
                case OP_LE:  tac_op = TAC_LE;  break;
                case OP_GE:  tac_op = TAC_GE;  break;
                case OP_EQ:  tac_op = TAC_EQ;  break;
                case OP_NE:  tac_op = TAC_NE;  break;
                default:     tac_op = TAC_ADD; break;
            }

            TACOperand temp = tac_new_temp(prog, node->inferred_type);
            tac_emit(prog, tac_op, temp, left, right);
            return temp;
        }

        default:
            return operand_none();
    }
}

static void gen_stmt(TACProgram *prog, const ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM: {
            for (size_t i = 0; i < node->data.program.stmt_count; i++) {
                gen_stmt(prog, node->data.program.statements[i]);
            }
            break;
        }

        case NODE_BLOCK: {
            for (size_t i = 0; i < node->data.block.stmt_count; i++) {
                gen_stmt(prog, node->data.block.statements[i]);
            }
            break;
        }

        case NODE_VAR_DECL: {
            if (node->data.var_decl.init_expr) {
                TACOperand val = gen_expr(prog, node->data.var_decl.init_expr);
                TACOperand var = operand_var(node->data.var_decl.var_name, node->data.var_decl.var_type);
                tac_emit(prog, TAC_ASSIGN, var, val, operand_none());
            }
            break;
        }

        case NODE_ASSIGN: {
            TACOperand val = gen_expr(prog, node->data.assign.expr);
            TACOperand var = operand_var(node->data.assign.var_name, node->inferred_type);
            tac_emit(prog, TAC_ASSIGN, var, val, operand_none());
            break;
        }

        case NODE_IF: {
            TACOperand cond = gen_expr(prog, node->data.if_stmt.condition);
            TACOperand l_else = tac_new_label(prog);
            TACOperand l_end = node->data.if_stmt.else_branch ? tac_new_label(prog) : l_else;

            tac_emit(prog, TAC_IF_FALSE, operand_none(), cond, l_else);
            gen_stmt(prog, node->data.if_stmt.then_branch);

            if (node->data.if_stmt.else_branch) {
                tac_emit(prog, TAC_GOTO, operand_none(), l_end, operand_none());
                tac_emit(prog, TAC_LABEL, l_else, operand_none(), operand_none());
                gen_stmt(prog, node->data.if_stmt.else_branch);
                tac_emit(prog, TAC_LABEL, l_end, operand_none(), operand_none());
            } else {
                tac_emit(prog, TAC_LABEL, l_else, operand_none(), operand_none());
            }
            break;
        }

        case NODE_WHILE: {
            TACOperand l_start = tac_new_label(prog);
            TACOperand l_end = tac_new_label(prog);

            tac_emit(prog, TAC_LABEL, l_start, operand_none(), operand_none());
            TACOperand cond = gen_expr(prog, node->data.while_stmt.condition);
            tac_emit(prog, TAC_IF_FALSE, operand_none(), cond, l_end);

            gen_stmt(prog, node->data.while_stmt.body);

            tac_emit(prog, TAC_GOTO, operand_none(), l_start, operand_none());
            tac_emit(prog, TAC_LABEL, l_end, operand_none(), operand_none());
            break;
        }

        default:
            break;
    }
}

void tac_generate(TACProgram *prog, const ASTNode *ast_root) {
    if (!prog || !ast_root) return;
    gen_stmt(prog, ast_root);
}

void tac_instr_print(const TACInstr *instr) {
    if (!instr) return;

    char r[64], a1[64], a2[64];
    operand_to_string(&instr->result, r, sizeof(r));
    operand_to_string(&instr->arg1, a1, sizeof(a1));
    operand_to_string(&instr->arg2, a2, sizeof(a2));

    switch (instr->op) {
        case TAC_LABEL:
            printf("%-10s:\n", r);
            break;
        case TAC_GOTO:
            printf("    goto %s\n", a1);
            break;
        case TAC_IF_FALSE:
            printf("    ifFalse %s goto %s\n", a1, a2);
            break;
        case TAC_IF_TRUE:
            printf("    ifTrue %s goto %s\n", a1, a2);
            break;
        case TAC_ASSIGN:
            printf("    %-6s = %s\n", r, a1);
            break;
        case TAC_ADD:
            printf("    %-6s = %s + %s\n", r, a1, a2);
            break;
        case TAC_SUB:
            printf("    %-6s = %s - %s\n", r, a1, a2);
            break;
        case TAC_MUL:
            printf("    %-6s = %s * %s\n", r, a1, a2);
            break;
        case TAC_DIV:
            printf("    %-6s = %s / %s\n", r, a1, a2);
            break;
        case TAC_MOD:
            printf("    %-6s = %s %% %s\n", r, a1, a2);
            break;
        case TAC_LT:
            printf("    %-6s = %s < %s\n", r, a1, a2);
            break;
        case TAC_GT:
            printf("    %-6s = %s > %s\n", r, a1, a2);
            break;
        case TAC_LE:
            printf("    %-6s = %s <= %s\n", r, a1, a2);
            break;
        case TAC_GE:
            printf("    %-6s = %s >= %s\n", r, a1, a2);
            break;
        case TAC_EQ:
            printf("    %-6s = %s == %s\n", r, a1, a2);
            break;
        case TAC_NE:
            printf("    %-6s = %s != %s\n", r, a1, a2);
            break;
    }
}

void tac_program_print(const TACProgram *prog, const char *title) {
    if (!prog) return;

    printf("========================================\n");
    printf("%s (%zu instructions)\n", title ? title : "THREE-ADDRESS CODE", prog->count);
    printf("========================================\n");

    TACInstr *curr = prog->head;
    while (curr) {
        tac_instr_print(curr);
        curr = curr->next;
    }

    printf("========================================\n\n");
}
