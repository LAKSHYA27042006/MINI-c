// Test: Scopes, declarations, and type compatibility
int g = 100;
float pi = 3.14;

{
    int local_var = 10;
    float local_res = local_var + pi;
}

int final_val = g + 50;
