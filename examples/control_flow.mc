// ============================================================
// MiniC Control Flow & Scoping Demonstration
// ============================================================
// Demonstrates:
// 1. if-else conditional branches
// 2. Block scoping with local variable declarations
// 3. while loop iteration
// ============================================================

int x = 15;
int y = 0;

if (x > 10) {
    int multiplier = 3;
    y = x * multiplier;
} else {
    y = 0;
}

while (y > 10) {
    y = y - 5;
}
