#include "usb_commands.h"

// Delegate that gets current RGB from RGB module.
static void (*get_rgb)(color_rgb_t*);
// Special struct that holds USB data.
usb_data_t* usb_data;
// Message between submodules size.
#define USBC_BUFF_MESSAGE_SIZE 512

// Parses string to <count> numbers.
bool parse_code(char *word, int *arr, int count)
{
    int i = 0;
    char *pot_nums = strtok(word, " ");
    // To remove first word
    pot_nums = strtok(NULL, " ");

    while (pot_nums != NULL)
    {
        if (i >= count)
            return false;
        arr[i++] = atoi(pot_nums);
        pot_nums = strtok(NULL, " ");
    }

    return i == count;
}

// Parses prefix from command.
usb_command_t parse_prefix(char *text)
{
    if (strncmp(text, "CURR", 4) == 0)
        return USB_COM_GET_RGB;
    if (strncmp(text, "RGB", 3) == 0)
        return USB_COM_RGB;
    if (strncmp(text, "HSV", 3) == 0)
        return USB_COM_HSV;
    if (strncmp(text, "HELP", 4) == 0)
        return USB_COM_HELP;

    return USB_COM_UNKNOWN;
}

// Processes commands that are not color-based.
static void process_other_command(char* text_buff, usb_command_t command)
{
    color_rgb_t clr;
    switch (command)
    {
    case USB_COM_GET_RGB:
        get_rgb(&clr);
        NRF_LOG_INFO("[USB RX] Requested RGB, current: R: %ld, G: %ld, B: %ld.", clr.r, clr.g, clr.b);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nRGB current: R: %ld, G: %ld, B: %ld.\r\n", clr.r, clr.g, clr.b);
        return;

    case USB_COM_HELP:
        NRF_LOG_INFO("[USB RX] HELP requested.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n==Commands==\r\nRGB <rrr> <ggg> <bbb>\r\n- Changes RGB. Color codes (rrr, ggg, bbb) should be between 0 and 255.\
                \r\nHSV <hhh> <sss> <vvv>\r\n- Changes HSV. Color code hhh should be between 0 and 360, sss and vvv - between 0 and 100.\
                \r\nCURR\r\n- Gets current RGB color.\r\n==End==\r\n");
        return;

    case USB_COM_UNKNOWN:
        NRF_LOG_INFO("[USB RX] Typed Enter. Unknown sequence.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nUnknown command. Type \"HELP\" to list available commands.\r\n");
        return;

    default:
        return;
    }
}

// Processes color command to print certain values.
static void process_color_command(char* text_buff, usb_command_t command, char *command_buff)
{
    int count;

    switch (command)
    {
    case USB_COM_RGB:
        count = 3;
        break;

    case USB_COM_HSV:
        count = 3;
        break;

    default:
        return;
    }

    bool ok;
    int nums[count];
    ok = parse_code(command_buff, nums, count);

    if (!ok)
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nWrong color sequence.\r\n");
        return;
    }

    switch (command)
    {
    case USB_COM_RGB:
        NRF_LOG_INFO("[USB RX] R: %d, G: %d, B: %d.", nums[0], nums[1], nums[2]);
        if (!helper_is_inbetween(nums[0], 0, 255))
        {
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nRed color code is invalid.\r\n");
            return;
        }
        if (!helper_is_inbetween(nums[1], 0, 255))
        {
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nGreen color code is invalid.\r\n");
            return;
        }
        if (!helper_is_inbetween(nums[2], 0, 255))
        {
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nBlue color code is invalid.\r\n");
            return;
        }
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nRGB color set to R: %d, G: %d, B: %d.\r\n", nums[0], nums[1], nums[2]);
        break;

    case USB_COM_HSV:
        NRF_LOG_INFO("[USB RX] H: %d, S: %d, V: %d.", nums[0], nums[1], nums[2]);
        if (!helper_is_inbetween(nums[0], 0, 360))
        {
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nHue is invalid.\r\n");
            return;
        }
        if (!helper_is_inbetween(nums[0], 0, 100))
        {
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nSaturation is invalid.\r\n");
            return;
        }
        if (!helper_is_inbetween(nums[0], 0, 100))
        {
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nBrightness is invalid.\r\n");
            return;
        }
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\nHSV color set to H: %d, S: %d, V: %d.\r\n", nums[0], nums[1], nums[2]);
        break;

    default:
        break;
    }
    usb_data->field1 = nums[0];
    usb_data->field2 = nums[1];
    usb_data->field3 = nums[2];
    usb_data->usb_color_command = command;
}

void usbc_process_command(char *buff, char *command_buff)
{
    usb_command_t command = parse_prefix(command_buff);
    switch (command)
    {
    case USB_COM_RGB:
    case USB_COM_HSV:
        process_color_command(buff, command, command_buff);
        return;
    default:
        process_other_command(buff, command);
        return;
    }

}

void usbc_init(usb_data_t *usbd, void (*action)(color_rgb_t*))
{
    get_rgb = action;
    usb_data = usbd;
}