#include "usb.h"

static char m_rx_buffer[READ_SIZE];

#define BUF_SUZE 16
static char command_buff[BUF_SUZE];
int num = 0;

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

static ret_code_t print_usb_message(const char *msg)
{
    return app_usbd_cdc_acm_write(&usb_cdc_acm, msg, strlen(msg));
}

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


static ret_code_t process_color_command(command_type_t command)
{
    int *nums;
    int count;
    bool ok;
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
    }
    nums = (int *)malloc(count * sizeof(int));
    ok = parse_code(command_buff, nums, count);

    if (!ok)
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax.");
        return print_usb_message("\r\nWrong color sequence.\r\n");
    }

    char buff[64];
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
        break;
    }
    free(nums);
    return print_usb_message(buff);
    return print_usb_message("\r\nSomething has gone wrong.\r\n");
}


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
        /* case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
         {
             break;
         }*/
    /*case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
    {
        // NRF_LOG_INFO("[TX] done");
        break;
    }*/
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
                    \r\nChange HSV by typing:\r\nHSV <hhh> <sss> <vvv>\r\nColor code hhh should be between 0 and 360, sss and vvv - between 0 and 100.\r\n");
                }
                else
                {
                    NRF_LOG_INFO("[USB RX] Typed Enter. Unknown sequence.");
                    ret = print_usb_message("\r\nUnknown command. Type \"help\" for list of available commands.\r\n");
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

            /* Fetch data until internal buffer is empty */
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

void usb_init()
{
    logs_init();
    NRF_LOG_INFO("Starting up the test project with USB logging");
    app_usbd_class_inst_t const *class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);
}

void usb_process()
{
    if (!app_usbd_event_queue_process())
    {
        logs_log_process();
    }
}
