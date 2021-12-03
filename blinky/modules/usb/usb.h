#ifndef MY_MODULE_USB
#define MY_MODULE_USB

#include "log.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"

#include <string.h>

/* Make sure that they don't intersect with LOG_BACKEND_USB_CDC_ACM */
#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

// USB module command types.
typedef enum
{
    // If there was RGB command.
    USB_COM_RGB,
    // If there was HSV command.
    USB_COM_HSV
} command_type_t;

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
    command_type_t usb_color_command;
} usb_data_t;

// Inits USB module. Need to pass usb_data_t pointer.
void usb_init(usb_data_t *usbd);

// Processes USB, need to be executed all the time.
void usb_process();

#endif