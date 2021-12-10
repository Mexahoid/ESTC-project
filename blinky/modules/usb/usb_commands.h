#ifndef MY_MODULE_USB_COMMANDS
#define MY_MODULE_USB_COMMANDS

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "helpers.h"

#include "log.h"
#include "color.h"


// USB module command types.
typedef enum
{
    // If there was RGB command.
    USB_COM_RGB,
    // If there was HSV command.
    USB_COM_HSV,
    // If there was CURR command.
    USB_COM_GET_RGB,
    // Wrong command.
    USB_COM_UNKNOWN,
    // If there was HELP command.
    USB_COM_HELP,
    // If there was ADDRGB command.
    USB_COM_ADD_RGB,
    // If there was ADDCURR command.
    USB_COM_ADD_CURR,
    // If there was DEL command.
    USB_COM_DEL,
    // If there was SET command.
    USB_COM_SET,
    // If there was LIST command.
    USB_COM_LIST

} usb_command_t;

// Structure holding USB input data.
typedef struct
{
    // First value;
    int32_t field1;
    // Second value;
    int32_t field2;
    // Third value;
    int32_t field3;
    // USB color mode.
    usb_command_t usb_color_command;
} usb_data_t;

/**
 * @brief Inits USB commands module.
 *
 * @param usbd             USB data pointer
 * @param action           Delegate for getting color from color module
 * */
void usbc_init(usb_data_t *usbd, void (*action)(color_rgb_t*));

/**
 * @brief Processes USB command and writes message to buff.
 *
 * @param buff             Buffer for messages
 * @param command_buff     Buffer with command contents
 * @param buff_msg_size    Buffer length
 * */
void usbc_process_command(char *buff, char *command_buff, int buff_msg_size);

#endif