#include "helpers.h"

int helper_min(int a, int b)
{
     return ((a <= b) ? a : b);
}
int helper_max(int a, int b)
{
    return ((a >= b) ? a : b);
}


int helper_clamp(int input, int min, int max)
{
    if (input > max)
        return max;
    if (input < min)
        return min;
    return input;
}

void helper_increment_circular(int *input, int *incr, int min, int max)
{
    *input += *incr;
    if (*input >= max || *input <= min)
        *incr *= -1;
}