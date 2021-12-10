#ifndef MY_MODULE_USBCCU
#define MY_MODULE_USBCCU

#include <string.h>
#include <stdio.h>
#include "helpers.h"


typedef enum
{
    USBCCU_OK,

    USBCCU_INVALID_RED,
    USBCCU_INVALID_GREEN,
    USBCCU_INVALID_BLUE,

    USBCCU_INVALID_HUE,
    USBCCU_INVALID_SAT,
    USBCCU_INVALID_BRI,

    USBCCU_INVALID_INTEGER,

    USBCCU_NAME_TOO_LONG
} usbccu_error_t;


/**
 * @brief Checks whether all numbers are valid HSV numbers.
 *
 * @param nums              Input array of values
 * @return usbccu_error_t
*/

usbccu_error_t usbccu_check_hsv(const int* const nums);

/**
 * @brief Checks whether all numbers are valid RGB numbers.
 *
 * @param nums              Input array of values
 * @return usbccu_error_t
*/
usbccu_error_t usbccu_check_rgb(const int* const nums);

/**
 * @brief Checks whether all of the needed values are integers.
 *
 * @param arr               Input array of strings
 * @param nums              Output array of integers
 * @param count             Count of needed integers
 * @return usbccu_error_t
*/
usbccu_error_t usbccu_check_ints(const char** const arr, int* const nums, int count);

/**
 * @brief Checks whether name length is less than max.
 *
 * @param name              Color name
 * @param max               Max name length
 * @return usbccu_error_t
*/
usbccu_error_t usbccu_check_name(const char* const name, int max);

#endif