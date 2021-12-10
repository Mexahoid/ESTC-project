#ifndef MY_MODULE_USB_COMMANDS
#define MY_MODULE_USB_COMMANDS

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "helpers.h"

#include "log.h"
#include "color.h"


// USB module color types.
typedef enum
{
    // If there was RGB command.
    USB_COLOR_RGB,
    // If there was HSV command.
    USB_COLOR_HSV,
} usb_color_t;

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
    usb_color_t usb_color;
} usb_data_t;

/**
 * @brief Inits USB commands module.
 *
 * @param usbd             USB data pointer
 * @param action           Delegate for getting color from color module
 * */
void usbc_init(usb_data_t * const usbd, void (*action)(color_rgb_t*));

/**
 * @brief Processes USB command and writes message to buff.
 *
 * @param buff             Buffer for messages
 * @param command_buff     Buffer with command contents
 * @param buff_msg_size    Buffer length
 * */
void usbc_process_command(char * const buff, char * const command_buff, int buff_msg_size);

#endif