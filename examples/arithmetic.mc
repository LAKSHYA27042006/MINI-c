// ============================================================
// MiniC Arithmetic & Operator Precedence Demonstration
// ============================================================
// Demonstrates:
// 1. Correct operator precedence (* and / over + and -)
// 2. Parentheses grouping overriding default precedence
// 3. Modulo operator (%) on integers
// ============================================================

int a = 10;
int b = 20;
int c = 5;

// Must parse as: a + (b * c) = 10 + 100 = 110
int res1 = a + b * c;

// Must parse as: (a + b) * c = 30 * 5 = 150
int res2 = (a + b) * c;

// Nested expression with division and modulo
int res3 = (a + b) / (c - 2) % 4;
