#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "tac.h"

typedef struct {
    int original_instruction_count;
    int optimized_instruction_count;
    int instructions_removed;
    int constant_folding_count;
    int algebraic_simplification_count;
    int constant_propagation_count;
    int temp_elimination_count;
    int dead_code_count;
    int unreachable_code_count;
    double reduction_percentage;
} OptimizationStats;

/* Initialize stats */
void optimization_stats_init(OptimizationStats *stats);

/* Run peephole optimization on TAC program */
TACProgram *optimize_tac(const TACProgram *input_prog, OptimizationStats *stats);

/* Print optimization report */
void optimization_report_print(const OptimizationStats *stats);

#endif /* OPTIMIZER_H */
