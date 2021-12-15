#include "usb_commands_color_utils.h"

usbccu_error_t usbccu_check_hsv(const int* const nums)
{
    if (!helper_is_inbetween(nums[0], 0, 360))
        return USBCCU_INVALID_HUE;

    if (!helper_is_inbetween(nums[1], 0, 100))
        return USBCCU_INVALID_SAT;

    if (!helper_is_inbetween(nums[2], 0, 100))
        return USBCCU_INVALID_BRI;

    return USBCCU_OK;
}

usbccu_error_t usbccu_check_rgb(const int* const nums)
{
    if (!helper_is_inbetween(nums[0], 0, 255))
        return USBCCU_INVALID_RED;

    if (!helper_is_inbetween(nums[1], 0, 255))
        return USBCCU_INVALID_GREEN;

    if (!helper_is_inbetween(nums[2], 0, 255))
        return USBCCU_INVALID_BLUE;

    return USBCCU_OK;
}

usbccu_error_t usbccu_check_ints(const char** const arr, int* const nums, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (sscanf(arr[i], "%d", &nums[i]) == 0)
            return USBCCU_INVALID_INTEGER;
    }
    return USBCCU_OK;
}

usbccu_error_t usbccu_check_name(const char* const name, int max)
{
    if (strlen(name) > max)
        return USBCCU_NAME_TOO_LONG;
    return USBCCU_OK;
}

usbccu_error_t usbccu_get_args(const char * const word, char * const buff, const char ** const arr, int count)
{
    int i = 0;
    memset(buff, 0, strlen(word));
    strcpy(buff, word);
    char *pot_nums = strtok(buff, " ");

    // To remove first word
    pot_nums = strtok(NULL, " ");

    while (pot_nums != NULL)
    {
        if (i >= count)
            return USBCCU_INVALID_ARGS;
        arr[i++] = pot_nums;
        pot_nums = strtok(NULL, " ");
    }
    return i == count ? USBCCU_OK : USBCCU_INVALID_ARGS;
}

usbccu_error_t usbccu_are_names_equal(const char * const name1, const char * const name2)
{
    if (strcmp(name1, name2) == 0)
        return USBCCU_NAME_EXISTS;
    return USBCCU_OK;
}