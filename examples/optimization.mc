// ============================================================
// MiniC Peephole Optimization Showcase
// ============================================================
// Demonstrates all key optimization rules:
// 1. Constant Folding: 3 + 4 -> 7, 10 * 5 -> 50
// 2. Algebraic Simplification: a + 0 -> a, b * 1 -> b, c * 0 -> 0
// 3. Constant Propagation: x = 10; y = x + 5 -> y = 10 + 5 -> 15
// 4. Redundant Temporary Elimination: direct assignments
// 5. Dead Code Elimination: overwritten stores
// ============================================================

// Rule 1: Constant Folding
int c1 = 3 + 4;
int c2 = 10 * 5;

// Rule 2: Algebraic Simplification
int a = 25;
int s1 = a + 0;
int s2 = a * 1;
int s3 = a * 0;
int s4 = a - 0;

// Rule 3: Constant Propagation leading to further folding
int p1 = 10;
int p2 = p1 + 5;

// Rule 5: Dead code (first assignment is immediately overwritten)
int dead = 100;
dead = 200;
