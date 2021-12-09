#include "usb_commands_color_utils.h"

// Message length of buffer.
static int buff_msg_size = 1024;

void usbccu_init(int size)
{
    buff_msg_size = size;
}

bool usbccu_check_hsv(int* nums, char* text_buff)
{
    if (!helper_is_inbetween(nums[0], 0, 360))
    {
        NRF_LOG_INFO("[USB RX] Hue invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Hue is invalid.\r\n");
        return false;
    }
    if (!helper_is_inbetween(nums[1], 0, 100))
    {
        NRF_LOG_INFO("[USB RX] Sat invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Saturation is invalid.\r\n");
        return false;
    }
    if (!helper_is_inbetween(nums[2], 0, 100))
    {
        NRF_LOG_INFO("[USB RX] Bri invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Brightness is invalid.\r\n");
        return false;
    }
    NRF_LOG_INFO("[USB RX] HSV color set to H: %d, S: %d, V: %d.", nums[0], nums[1], nums[2]);
    snprintf(text_buff, buff_msg_size, "\r\n>> HSV color set to H: %d, S: %d, V: %d.\r\n", nums[0], nums[1], nums[2]);
    return true;
}

bool usbccu_check_rgb(int* nums, char* text_buff)
{
    if (!helper_is_inbetween(nums[0], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Red invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Red color code is invalid.\r\n");
        return false;
    }
    if (!helper_is_inbetween(nums[1], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Green invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Green color code is invalid.\r\n");
        return false;
    }
    if (!helper_is_inbetween(nums[2], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Blue invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Blue color code is invalid.\r\n");
        return false;
    }
    return true;
}

bool usbccu_check_ints(char** arr, int* nums, int count, char* text_buff)
{
    for (int i = 0; i < count; i++)
    {
        if (sscanf(arr[i], "%d", &nums[i]) == 0)
        {
            NRF_LOG_INFO("[USB RX] Wrong data: %s", arr[i]);
            snprintf(text_buff, buff_msg_size, "\r\n> Wrong data passed: %s.\r\n", arr[i]);
            return false;
        }
    }
    return true;
}

bool usbccu_check_name(char* name, int max, char* text_buff)
{
    if (strlen(name) > max)
    {
        NRF_LOG_INFO("[USB RX] Name is too long: %s", name);
        snprintf(text_buff, buff_msg_size, "\r\n> The name is too long, can't exceed %d.\r\n", max);
        return false;
    }
    return true;
}