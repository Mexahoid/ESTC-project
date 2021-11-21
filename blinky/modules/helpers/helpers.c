#include "helpers.h"

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