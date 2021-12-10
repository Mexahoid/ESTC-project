#include "usb.h"
#include <ctype.h>

#define READ_SIZE 1
static char m_rx_buffer[READ_SIZE];
#define BUF_SUZE 32
// Buffer for a typed command.
static char command_buff[BUF_SUZE];
// Command_buff index pointer.
static int num = 0;
// Command parser.
static void (*parse_command)(char*, char*);


static void ev_handler(app_usbd_class_inst_t const *p_inst,
                       app_usbd_cdc_acm_user_event_t event);

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

// Prints string message to the port.
static ret_code_t print_usb_message(const char *msg)
{
    return app_usbd_cdc_acm_write(&usb_cdc_acm, msg, strlen(msg));
}

// Processes color command to print certain values.
static ret_code_t process_command()
{
    char res[USB_BUFF_MESSAGE_SIZE];
    memset(res, 0, USB_BUFF_MESSAGE_SIZE);
    parse_command(res, command_buff);
    memset(command_buff, 0, BUF_SUZE);
    num = 0;
    return print_usb_message(res);
}

// USB event handler.
static void ev_handler(app_usbd_class_inst_t const *p_inst,
                       app_usbd_cdc_acm_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
    {
        ret_code_t ret;
        ret = app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
    {
        ret_code_t ret;
        do
        {
            size_t size = app_usbd_cdc_acm_rx_size(&usb_cdc_acm);
            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n')
            {
                ret = process_command();
            }
            else
            {
                if (isprint((uint8_t)(m_rx_buffer[0])) != 0)
                {
                    NRF_LOG_INFO("[USB RX] Val: %c, size: %d.", m_rx_buffer[0], size);
                    if (num < BUF_SUZE - 1)
                    {
                        ret = print_usb_message(m_rx_buffer);
                        command_buff[num++] = m_rx_buffer[0];
                    }
                }
                else
                    NRF_LOG_INFO("[USB RX] Unknown symbol.");

            }
            ret = app_usbd_cdc_acm_read(&usb_cdc_acm,
                                        m_rx_buffer,
                                        READ_SIZE);
        } while (ret == NRF_SUCCESS);

        break;
    }
    default:
        break;
    }
}


void usb_init(void (*action)(char*, char*))
{
    logs_init();
    parse_command = action;
    app_usbd_class_inst_t const *class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);
}

void usb_process()
{
    if (!app_usbd_event_queue_process())
    {
        logs_process();
    }
}
