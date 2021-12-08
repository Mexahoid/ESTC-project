#include "usb.h"

#define READ_SIZE 1
static char m_rx_buffer[READ_SIZE];
#define BUF_SUZE 16
// Buffer for a typed command.
static char command_buff[BUF_SUZE];
// command_buff index pointer.
static int num = 0;

static void (*get_rgb)(color_rgb_t*);

// Special struct that holds USB data.
usb_data_t* usb_data;

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

// Parses string to <count> numbers.
static bool parse_code(char *word, int *arr, int count)
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

// Processes color command to print certain values.
static ret_code_t process_color_command(command_type_t command)
{
    int count;
    bool ok;

    char buff[64];
    switch (command)
    {
    case USB_COM_RGB:
        NRF_LOG_INFO("[USB RX] RGB sequence found.");
        count = 3;
        break;
    case USB_COM_HSV:
        NRF_LOG_INFO("[USB RX] HSV sequence found.");
        count = 3;
        break;
    case USB_COM_GET_RGB:
        NRF_LOG_INFO("[USB RX] Requested RGB.");
        color_rgb_t clr;
        get_rgb(&clr);
        NRF_LOG_INFO("[USB RX] Current RGB. R: %ld, G: %ld, B: %ld.", clr.r, clr.g, clr.b);
        snprintf(buff, 64, "\r\nRGB current: R: %ld, G: %ld, B: %ld.\r\n", clr.r, clr.g, clr.b);
        return print_usb_message(buff);
    }
    int nums[count];
    ok = parse_code(command_buff, nums, count);

    if (!ok)
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax.");
        return print_usb_message("\r\nWrong color sequence.\r\n");
    }

    switch (command)
    {
    case USB_COM_RGB:
        NRF_LOG_INFO("[USB RX] R: %d, G: %d, B: %d.", nums[0], nums[1], nums[2]);
        if (nums[0] < 0 || nums[0] > 255)
        {
            free(nums);
            return print_usb_message("\r\nRed color code is invalid.\r\n");
        }
        if (nums[1] < 0 || nums[1] > 255)
        {
            free(nums);
            return print_usb_message("\r\nGreen color code is invalid.\r\n");
        }
        if (nums[2] < 0 || nums[2] > 255)
        {
            free(nums);
            return print_usb_message("\r\nBlue color code is invalid.\r\n");
        }
        snprintf(buff, 64, "\r\nRGB color set to R: %d, G: %d, B: %d.\r\n", nums[0], nums[1], nums[2]);
        usb_data->field1 = nums[0];
        usb_data->field2 = nums[1];
        usb_data->field3 = nums[2];
        break;

    case USB_COM_HSV:
        NRF_LOG_INFO("[USB RX] H: %d, S: %d, V: %d.", nums[0], nums[1], nums[2]);
        if (nums[0] < 0 || nums[0] > 360)
        {
            free(nums);
            return print_usb_message("\r\nHue is invalid.\r\n");
        }
        if (nums[1] < 0 || nums[1] > 100)
        {
            free(nums);
            return print_usb_message("\r\nSaturation is invalid.\r\n");
        }
        if (nums[2] < 0 || nums[2] > 100)
        {
            free(nums);
            return print_usb_message("\r\nBrightness is invalid.\r\n");
        }
        snprintf(buff, 64, "\r\nHSV color set to H: %d, S: %d, V: %d.\r\n", nums[0], nums[1], nums[2]);
        usb_data->field1 = nums[0];
        usb_data->field2 = nums[1];
        usb_data->field3 = nums[2];
        break;

    default:
    break;
    }
    usb_data->usb_color_command = command;
    return print_usb_message(buff);
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
                if (num < BUF_SUZE)
                {
                    command_buff[num] = '\0';
                }

                if (strncmp(command_buff, "CURR", 4) == 0)
                {
                    ret = process_color_command(USB_COM_GET_RGB);
                } else
                if (strncmp(command_buff, "RGB", 3) == 0)
                {
                    ret = process_color_command(USB_COM_RGB);
                } else
                if (strncmp(command_buff, "HSV", 3) == 0)
                {
                    ret = process_color_command(USB_COM_HSV);
                } else
                if (strncmp(command_buff, "help", 4) == 0)
                {
                    NRF_LOG_INFO("[USB RX] Typed \"help\"");
                    ret = print_usb_message("\r\nChange RGB by typing:\r\nRGB <rrr> <ggg> <bbb>\r\nColor codes (rrr, ggg, bbb) should be between 0 and 255.\r\n\
                    \r\nChange HSV by typing:\r\nHSV <hhh> <sss> <vvv>\r\nColor code hhh should be between 0 and 360, sss and vvv - between 0 and 100.\r\n\
                    \r\nGet current RGB color by typing:\r\nCURR\r\n");
                }
                else
                {
                    NRF_LOG_INFO("[USB RX] Typed Enter. Unknown sequence.");
                    ret = print_usb_message("\r\nUnknown command. Type \"help\" to list available commands.\r\n");
                }
                num = 0;
            }
            else
            {
                NRF_LOG_INFO("[USB RX] Val: %c, size: %d.", m_rx_buffer[0], size);
                ret = print_usb_message(m_rx_buffer);

                if (num < BUF_SUZE)
                {
                    command_buff[num++] = m_rx_buffer[0];
                    if (num == BUF_SUZE - 1)
                        command_buff[num++] = '\0';
                }
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


void usb_init(usb_data_t *usbd, void (*action)(color_rgb_t*))
{
    logs_init();
    get_rgb = action;
    usb_data = usbd;
    NRF_LOG_INFO("Starting up the test project with USB logging");
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
