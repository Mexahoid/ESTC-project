#ifndef MY_MODULE_USB_COMMANDS
#define MY_MODULE_USB_COMMANDS

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "helpers.h"

#include "log.h"
#include "color.h"

// Message between submodules size.
#define USBC_BUFF_MESSAGE_SIZE 512

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
    USB_COM_HELP
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

// Inits USB commands module.
void usbc_init(usb_data_t *usbd, void (*action)(color_rgb_t*));

// Processes USB command and writes message to buff.
void usbc_process_command(char *buff, char *command_buff);

#endif