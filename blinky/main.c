/**
 * Copyright 2021 Evgeniy Morozov
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
*/

/** @file
 *
 * @defgroup usb_logging_example_main main.c
 * @{
 * @ingroup usb_logging_example
 * @brief Example logging over USB (for nrf52840 dongle or DK).
 *
 * This example contains all the necessary code to build logging over the USB.
 *
 * Example inverts LED colours every second and prints a message to the USB log.
 *
 * It is possible to configure USB stack manually by setting
 * LOG_BACKEND_USB_INIT_STACK to 0 or leave USB init to the stack by setting
 * LOG_BACKEND_USB_INIT_STACK to 1.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#include "nordic_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#define BLINK_DELAY_MS 500

typedef enum
{
    BUTTON1 = NRF_GPIO_PIN_MAP(1, 6),
} button_t;

typedef enum
{
    YELLOW = NRF_GPIO_PIN_MAP(0, 6),
    RED = NRF_GPIO_PIN_MAP(0, 8),
    GREEN = NRF_GPIO_PIN_MAP(1, 9),
    BLUE = NRF_GPIO_PIN_MAP(0, 12),
} led_t;


void init_button(button_t btn)
{
    nrf_gpio_cfg_input(btn, GPIO_PIN_CNF_PULL_Pullup);
}

bool is_button_pressed(button_t btn)
{
    return nrf_gpio_pin_read(btn) != 1;
}

void init_led(led_t led)
{
    nrf_gpio_cfg_output(led);
    nrf_gpio_pin_write(led, 1);
}

void change_led_state(led_t led)
{
    nrf_gpio_pin_toggle(led);
}

void delay_ms(int amount)
{
    nrf_delay_ms(amount);
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    NRF_LOG_INFO("Starting up the test project with USB logging");
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    led_t leds[] = {YELLOW, RED, GREEN, BLUE};
    int counts[] = {6, 6, 1, 3};

    for (int i = 0; i < ARRAY_SIZE(leds); i++)
    {
        init_led(leds[i]);
    }
    init_button(BUTTON1);

    int pos = 0;
    int counter = 1;
    int delay_counter = 0;
    bool flag = false;
    bool button_is_pressed = false;

    while (true)
    {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();


        if(!is_button_pressed(BUTTON1))
        {
            if (button_is_pressed)
            {
                NRF_LOG_INFO("Button released");
                LOG_BACKEND_USB_PROCESS();
                NRF_LOG_INFO("Current delay: %d out of %d", delay_counter, BLINK_DELAY_MS);
                LOG_BACKEND_USB_PROCESS();
                button_is_pressed = false;
            }
            continue;
        }
        if (!button_is_pressed)
        {
            NRF_LOG_INFO("Button pressed");
            LOG_BACKEND_USB_PROCESS();
            button_is_pressed = true;
        }
        if(delay_counter > BLINK_DELAY_MS)
        {
            change_led_state(leds[pos]);
            flag = !flag;
            if(!flag)
            {
                NRF_LOG_INFO("%d led blink: %d of %d", pos + 1, counter, counts[pos]);
                LOG_BACKEND_USB_PROCESS();
                counter++;
            }
            delay_counter = 0;
            if (counter > counts[pos])
            {
                counter = 1;
                pos++;
                pos %= ARRAY_SIZE(counts);
            }
        }

        delay_counter++;
        delay_ms(1);
    }
}