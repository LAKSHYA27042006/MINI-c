#include "optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void optimization_stats_init(OptimizationStats *stats) {
    if (!stats) return;
    memset(stats, 0, sizeof(OptimizationStats));
}

static bool is_const(const TACOperand *op) {
    return op && (op->type == OPERAND_CONST_INT || op->type == OPERAND_CONST_FLOAT);
}

static bool is_const_zero(const TACOperand *op) {
    if (!op) return false;
    if (op->type == OPERAND_CONST_INT) return op->val.int_val == 0;
    if (op->type == OPERAND_CONST_FLOAT) return op->val.float_val == 0.0;
    return false;
}

static bool is_const_one(const TACOperand *op) {
    if (!op) return false;
    if (op->type == OPERAND_CONST_INT) return op->val.int_val == 1;
    if (op->type == OPERAND_CONST_FLOAT) return op->val.float_val == 1.0;
    return false;
}

static size_t count_uses(const TACProgram *prog, const TACOperand *op) {
    if (!prog || !op) return 0;
    size_t count = 0;
    TACInstr *curr = prog->head;
    while (curr) {
        if (operand_equals(&curr->arg1, op)) count++;
        if (operand_equals(&curr->arg2, op)) count++;
        curr = curr->next;
    }
    return count;
}

/* 1. Unreachable Code Elimination */
static bool pass_unreachable_code(TACProgram *prog, OptimizationStats *stats) {
    bool changed = false;
    TACInstr *curr = prog->head;

    while (curr) {
        if (curr->op == TAC_GOTO) {
            while (curr->next && curr->next->op != TAC_LABEL) {
                TACInstr *dead = curr->next;
                tac_instr_remove(prog, dead);
                stats->unreachable_code_count++;
                changed = true;
            }
        }
        curr = curr->next;
    }

    return changed;
}

/* 2. Constant Propagation */
typedef struct {
    char name[64];
    TACOperand val;
} ConstBinding;

static bool pass_constant_propagation(TACProgram *prog, OptimizationStats *stats) {
    bool changed = false;
    ConstBinding bindings[128];
    int binding_count = 0;

    TACInstr *curr = prog->head;
    while (curr) {
        /* Control flow boundaries clear known constants */
        if (curr->op == TAC_LABEL || curr->op == TAC_GOTO ||
            curr->op == TAC_IF_FALSE || curr->op == TAC_IF_TRUE) {
            binding_count = 0;
            curr = curr->next;
            continue;
        }

        /* Check arg1 propagation */
        if (curr->arg1.type == OPERAND_VAR) {
            for (int i = 0; i < binding_count; i++) {
                if (strcmp(bindings[i].name, curr->arg1.name) == 0) {
                    curr->arg1 = bindings[i].val;
                    stats->constant_propagation_count++;
                    changed = true;
                    break;
                }
            }
        }

        /* Check arg2 propagation */
        if (curr->arg2.type == OPERAND_VAR) {
            for (int i = 0; i < binding_count; i++) {
                if (strcmp(bindings[i].name, curr->arg2.name) == 0) {
                    curr->arg2 = bindings[i].val;
                    stats->constant_propagation_count++;
                    changed = true;
                    break;
                }
            }
        }

        /* Update bindings on assignment */
        if (curr->result.type == OPERAND_VAR) {
            int found_idx = -1;
            for (int i = 0; i < binding_count; i++) {
                if (strcmp(bindings[i].name, curr->result.name) == 0) {
                    found_idx = i;
                    break;
                }
            }

            if (curr->op == TAC_ASSIGN && is_const(&curr->arg1)) {
                if (found_idx >= 0) {
                    bindings[found_idx].val = curr->arg1;
                } else if (binding_count < 128) {
                    strncpy(bindings[binding_count].name, curr->result.name, 63);
                    bindings[binding_count].name[63] = '\0';
                    bindings[binding_count].val = curr->arg1;
                    binding_count++;
                }
            } else {
                /* Target overwritten with non-constant: invalidate */
                if (found_idx >= 0) {
                    bindings[found_idx] = bindings[binding_count - 1];
                    binding_count--;
                }
            }
        }

        curr = curr->next;
    }

    return changed;
}

/* 3. Constant Folding */
static bool pass_constant_folding(TACProgram *prog, OptimizationStats *stats) {
    (void)prog;
    bool changed = false;
    TACInstr *curr = prog->head;

    while (curr) {
        if (is_const(&curr->arg1) && is_const(&curr->arg2)) {
            bool is_float_op = (curr->arg1.type == OPERAND_CONST_FLOAT || curr->arg2.type == OPERAND_CONST_FLOAT);

            /* Check division / modulo by zero */
            if (curr->op == TAC_DIV || curr->op == TAC_MOD) {
                if (is_const_zero(&curr->arg2)) {
                    curr = curr->next;
                    continue; /* Do not fold division by zero */
                }
            }

            if (!is_float_op) {
                long long v1 = curr->arg1.val.int_val;
                long long v2 = curr->arg2.val.int_val;
                long long res = 0;
                bool folded = true;

                switch (curr->op) {
                    case TAC_ADD: res = v1 + v2; break;
                    case TAC_SUB: res = v1 - v2; break;
                    case TAC_MUL: res = v1 * v2; break;
                    case TAC_DIV: res = v1 / v2; break;
                    case TAC_MOD: res = v1 % v2; break;
                    case TAC_LT:  res = (v1 < v2); break;
                    case TAC_GT:  res = (v1 > v2); break;
                    case TAC_LE:  res = (v1 <= v2); break;
                    case TAC_GE:  res = (v1 >= v2); break;
                    case TAC_EQ:  res = (v1 == v2); break;
                    case TAC_NE:  res = (v1 != v2); break;
                    default:      folded = false; break;
                }

                if (folded) {
                    curr->op = TAC_ASSIGN;
                    curr->arg1 = operand_const_int(res);
                    curr->arg2 = operand_none();
                    stats->constant_folding_count++;
                    changed = true;
                }
            } else {
                double v1 = (curr->arg1.type == OPERAND_CONST_FLOAT)
                            ? curr->arg1.val.float_val
                            : (double)curr->arg1.val.int_val;
                double v2 = (curr->arg2.type == OPERAND_CONST_FLOAT)
                            ? curr->arg2.val.float_val
                            : (double)curr->arg2.val.int_val;

                bool is_relational = (curr->op == TAC_LT || curr->op == TAC_GT ||
                                      curr->op == TAC_LE || curr->op == TAC_GE ||
                                      curr->op == TAC_EQ || curr->op == TAC_NE);

                if (is_relational) {
                    long long res = 0;
                    switch (curr->op) {
                        case TAC_LT: res = (v1 < v2); break;
                        case TAC_GT: res = (v1 > v2); break;
                        case TAC_LE: res = (v1 <= v2); break;
                        case TAC_GE: res = (v1 >= v2); break;
                        case TAC_EQ: res = (v1 == v2); break;
                        case TAC_NE: res = (v1 != v2); break;
                        default: break;
                    }
                    curr->op = TAC_ASSIGN;
                    curr->arg1 = operand_const_int(res);
                    curr->arg2 = operand_none();
                    stats->constant_folding_count++;
                    changed = true;
                } else if (curr->op != TAC_MOD) {
                    double res = 0.0;
                    switch (curr->op) {
                        case TAC_ADD: res = v1 + v2; break;
                        case TAC_SUB: res = v1 - v2; break;
                        case TAC_MUL: res = v1 * v2; break;
                        case TAC_DIV: res = v1 / v2; break;
                        default: break;
                    }
                    curr->op = TAC_ASSIGN;
                    curr->arg1 = operand_const_float(res);
                    curr->arg2 = operand_none();
                    stats->constant_folding_count++;
                    changed = true;
                }
            }
        }

        curr = curr->next;
    }

    return changed;
}

/* 4. Algebraic Simplification */
static bool pass_algebraic_simplification(TACProgram *prog, OptimizationStats *stats) {
    (void)prog;
    bool changed = false;
    TACInstr *curr = prog->head;

    while (curr) {
        if (curr->op == TAC_ADD) {
            if (is_const_zero(&curr->arg2)) {
                /* x = y + 0 => x = y */
                curr->op = TAC_ASSIGN;
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            } else if (is_const_zero(&curr->arg1)) {
                /* x = 0 + y => x = y */
                curr->op = TAC_ASSIGN;
                curr->arg1 = curr->arg2;
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            }
        } else if (curr->op == TAC_SUB) {
            if (is_const_zero(&curr->arg2)) {
                /* x = y - 0 => x = y */
                curr->op = TAC_ASSIGN;
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            } else if (operand_equals(&curr->arg1, &curr->arg2)) {
                /* x = y - y => x = 0 */
                curr->op = TAC_ASSIGN;
                curr->arg1 = operand_const_int(0);
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            }
        } else if (curr->op == TAC_MUL) {
            if (is_const_one(&curr->arg2)) {
                /* x = y * 1 => x = y */
                curr->op = TAC_ASSIGN;
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            } else if (is_const_one(&curr->arg1)) {
                /* x = 1 * y => x = y */
                curr->op = TAC_ASSIGN;
                curr->arg1 = curr->arg2;
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            } else if (is_const_zero(&curr->arg2)) {
                /* x = y * 0 => x = 0 */
                curr->op = TAC_ASSIGN;
                curr->arg1 = curr->arg2;
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            } else if (is_const_zero(&curr->arg1)) {
                /* x = 0 * y => x = 0 */
                curr->op = TAC_ASSIGN;
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            }
        } else if (curr->op == TAC_DIV) {
            if (is_const_one(&curr->arg2)) {
                /* x = y / 1 => x = y */
                curr->op = TAC_ASSIGN;
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            } else if (operand_equals(&curr->arg1, &curr->arg2) && !is_const_zero(&curr->arg2)) {
                /* x = y / y => x = 1 */
                curr->op = TAC_ASSIGN;
                curr->arg1 = operand_const_int(1);
                curr->arg2 = operand_none();
                stats->algebraic_simplification_count++;
                changed = true;
            }
        }

        curr = curr->next;
    }

    return changed;
}

/* 5. Redundant Temporary Elimination */
static bool pass_temporary_elimination(TACProgram *prog, OptimizationStats *stats) {
    bool changed = false;
    TACInstr *curr = prog->head;

    while (curr && curr->next) {
        TACInstr *next = curr->next;

        /* Pattern:
         * curr: t1 = ...
         * next: x = t1
         * where t1 is a temporary and is only used once in the entire program (here in next)
         */
        if (curr->result.type == OPERAND_TEMP &&
            next->op == TAC_ASSIGN &&
            next->arg1.type == OPERAND_TEMP &&
            operand_equals(&curr->result, &next->arg1)) {

            if (count_uses(prog, &curr->result) == 1) {
                /* Replace curr's target with next's target */
                curr->result = next->result;
                /* Remove next */
                tac_instr_remove(prog, next);
                stats->temp_elimination_count++;
                changed = true;
                continue;
            }
        }

        curr = curr->next;
    }

    return changed;
}

/* 6. Dead Code Elimination */
static bool pass_dead_code_elimination(TACProgram *prog, OptimizationStats *stats) {
    bool changed = false;
    TACInstr *curr = prog->head;

    while (curr) {
        TACInstr *next_instr = curr->next;

        /* A: Unused temporaries */
        if (curr->result.type == OPERAND_TEMP) {
            if (count_uses(prog, &curr->result) == 0) {
                tac_instr_remove(prog, curr);
                stats->dead_code_count++;
                changed = true;
                curr = next_instr;
                continue;
            }
        }

        /* B: Consecutive dead assignments to variables within the same basic block */
        if (curr->result.type == OPERAND_VAR && curr->op == TAC_ASSIGN) {
            TACInstr *scan = curr->next;
            bool is_dead = false;

            while (scan) {
                /* Stop at control flow boundaries */
                if (scan->op == TAC_LABEL || scan->op == TAC_GOTO ||
                    scan->op == TAC_IF_FALSE || scan->op == TAC_IF_TRUE) {
                    break;
                }

                /* If read anywhere, not dead */
                if (operand_equals(&scan->arg1, &curr->result) ||
                    operand_equals(&scan->arg2, &curr->result)) {
                    break;
                }

                /* If overwritten without prior read, curr is dead */
                if (operand_equals(&scan->result, &curr->result)) {
                    is_dead = true;
                    break;
                }

                scan = scan->next;
            }

            if (is_dead) {
                tac_instr_remove(prog, curr);
                stats->dead_code_count++;
                changed = true;
                curr = next_instr;
                continue;
            }
        }

        curr = next_instr;
    }

    return changed;
}

TACProgram *optimize_tac(const TACProgram *input_prog, OptimizationStats *stats) {
    if (!input_prog || !stats) return NULL;

    optimization_stats_init(stats);
    stats->original_instruction_count = (int)input_prog->count;

    TACProgram *opt_prog = tac_program_clone(input_prog);

    int max_passes = 10;
    int pass = 0;
    bool changed = false;

    do {
        changed = false;
        changed |= pass_unreachable_code(opt_prog, stats);
        changed |= pass_constant_propagation(opt_prog, stats);
        changed |= pass_constant_folding(opt_prog, stats);
        changed |= pass_algebraic_simplification(opt_prog, stats);
        changed |= pass_temporary_elimination(opt_prog, stats);
        changed |= pass_dead_code_elimination(opt_prog, stats);
        pass++;
    } while (changed && pass < max_passes);

    stats->optimized_instruction_count = (int)opt_prog->count;
    stats->instructions_removed = stats->original_instruction_count - stats->optimized_instruction_count;
    if (stats->original_instruction_count > 0) {
        stats->reduction_percentage = ((double)stats->instructions_removed /
                                       (double)stats->original_instruction_count) * 100.0;
    } else {
        stats->reduction_percentage = 0.0;
    }

    return opt_prog;
}

void optimization_report_print(const OptimizationStats *stats) {
    if (!stats) return;

    printf("========================================\n");
    printf("OPTIMIZATION REPORT\n");
    printf("========================================\n\n");
    printf("Original TAC instructions : %d\n", stats->original_instruction_count);
    printf("Optimized TAC instructions: %d\n\n", stats->optimized_instruction_count);
    printf("Instructions removed      : %d\n\n", stats->instructions_removed);
    printf("Constant folding          : %d\n", stats->constant_folding_count);
    printf("Algebraic simplification  : %d\n", stats->algebraic_simplification_count);
    printf("Constant propagation      : %d\n", stats->constant_propagation_count);
    printf("Temporary elimination     : %d\n", stats->temp_elimination_count);
    printf("Dead code elimination     : %d\n", stats->dead_code_count);
    printf("Unreachable code removal  : %d\n\n", stats->unreachable_code_count);
    printf("Reduction                 : %.2f%%\n\n", stats->reduction_percentage);
    printf("========================================\n\n");
}
