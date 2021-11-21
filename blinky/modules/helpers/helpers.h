#ifndef MY_MODULE_HELPER
#define MY_MODULE_HELPER

// Returns input if it lt max and gt min, otherwise returns boundary value.
int helper_clamp(int input, int min, int max);

void helper_increment_circular(int *input, int *incr, int min, int max);

#endif