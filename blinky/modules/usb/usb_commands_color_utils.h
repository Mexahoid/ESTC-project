#ifndef MY_MODULE_USBCCU
#define MY_MODULE_USBCCU

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "helpers.h"
#include "log.h"


/**
 * @brief Checks whether all numbers are valid HSV numbers.
 *
 * @param nums          Input array of values
 * @param text_buff     Text buffer that prints the message
 * @return true
 * @return false
*/
bool usbccu_check_hsv(int* nums, char* text_buff);

/**
 * @brief Checks whether all numbers are valid RGB numbers.
 *
 * @param nums          Input array of values
 * @param text_buff     Text buffer that prints the message
 * @return true
 * @return false
*/
bool usbccu_check_rgb(int* nums, char* text_buff);

/**
 * @brief Checks whether all of the needed values are integers.
 *
 * @param arr           Input array of strings
 * @param nums          Output array of integers
 * @param count         Count of needed integers
 * @param text_buff     Text buffer that prints the message
 * @return true
 * @return false
*/
bool usbccu_check_ints(char** arr, int* nums, int count, char* text_buff);

/**
 * @brief Inits USBCCU module.
 *
 * @param size          Buffer message size
*/
void usbccu_init(int size);

/**
 * @brief Checks whether name length is less than max.
 *
 * @param name          Color name
 * @param max           Max name length
 * @param text_buff     Text buffer that prints the message
 * @return true
 * @return false
*/
bool usbccu_check_name(char* name, int max, char* text_buff);

#endif