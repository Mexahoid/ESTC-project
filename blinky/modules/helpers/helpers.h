#ifndef MY_MODULE_HELPER
#define MY_MODULE_HELPER

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Returns min of two numbers.
 *
 * @param a                 First number
 * @param b                 Second number
 * @return int
 */
int helper_min(int a, int b);

/**
 * @brief Returns max of two numbers.
 *
 * @param a                 First number
 * @param b                 Second number
 * @return int
 */
int helper_max(int a, int b);

/**
 * @brief Returns input if it lt max and gt min, otherwise returns boundary value.
 *
 * @param input            Input value
 * @param min              Min value for clamping
 * @param max              Max value for clamping
 * @return int
 * */
int helper_clamp(int input, int min, int max);

/**
 * @brief Makes increment circular (it goes to max, then to min and again to max ad infinitum).
 *
 * @param input            Pointer to an input
 * @param incr             Pointer to an increment
 * @param min              Min value for incrementing
 * @param max              Max value for incrementing
 * */
void helper_increment_circular(int32_t* const input, int32_t* const incr, int min, int max);

/**
 * @brief Checks whether value is between min amd max.
 *
 * @param input             Input value
 * @param min               Min value
 * @param max               Max value
 * @return true
 * @return false
 */
bool helper_is_inbetween(int input, int min, int max);

#endif