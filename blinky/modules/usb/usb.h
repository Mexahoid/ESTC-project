#ifndef MY_MODULE_USB
#define MY_MODULE_USB

#include <string.h>

#include "color.h"
#include "usb_commands.h"

#include "log.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"


/* Make sure that they don't intersect with LOG_BACKEND_USB_CDC_ACM */
#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4




// Inits USB module. Need to pass usb_data_t pointer.
void usb_init(usb_data_t *usbd, void (*action)(color_rgb_t*));

// Processes USB, need to be executed all the time.
void usb_process();

#endif