#ifndef MY_MODULE_HELPER
#define MY_MODULE_HELPER

// Returns min of two numbers.
int helper_min(int a, int b);

// Returns max of two numbers.
int helper_max(int a, int b);

// Returns input if it lt max and gt min, otherwise returns boundary value.
int helper_clamp(int input, int min, int max);

// Makes increment circular (it goes to max, then to min and again to max ad infinitum).
void helper_increment_circular(int* const input, int* const incr, int min, int max);

#endif