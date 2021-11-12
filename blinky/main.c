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

#include "nrfx_systick.h"

#define BLINK_DELAY_MS 1000

#define PWM_FREQUENCY 1000

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



void pwm_modulate(int pwm_percentage, int delay, led_t led)
{
    bool is_on = false;
    // Percent of duty delay
    int time_percent = 0;
    nrfx_systick_state_t timestamp;
    while (time_percent < 100)
    {
        nrfx_systick_get(&timestamp);
        while (!nrfx_systick_test(&timestamp, delay))
            continue;

        if (time_percent < pwm_percentage && !is_on)
        {
            is_on = true;
            nrf_gpio_pin_write(led, 0);
        }

        if (time_percent >= pwm_percentage && is_on)
        {
            is_on = false;
            nrf_gpio_pin_write(led, 1);
        }
        time_percent++;
    }
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
    int leds_blink_counts[] = {6, 6, 1, 3};

    for (int i = 0; i < ARRAY_SIZE(leds); i++)
    {
        init_led(leds[i]);
    }
    init_button(BUTTON1);
    nrfx_systick_init();



    int led_index = 0;
    int led_blink_counter = 1;
    bool button_is_pressed = false;


    int pwm_percentage = 0;
    // X ms for 1%, 1000 ms -> 500 ms for 0-100% -> 5ms for 1% pwm duty
    int pwm_percent_delay_ms = BLINK_DELAY_MS / 100 / 2;
    int pwm_delay_ms = 0;

    int counter_ms = 0;

    // duty percent delay (in us) for pwm function, needs refactoring
    int pwm_duty_delay_us = 1000000 / 100 / PWM_FREQUENCY;

    while (true)
    {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();


        pwm_modulate(pwm_percentage, pwm_duty_delay_us, leds[led_index]);

        if(!is_button_pressed(BUTTON1))
        {
            if (button_is_pressed)
            {
                NRF_LOG_INFO("Button released");
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

        if(counter_ms >= BLINK_DELAY_MS)
        {
            NRF_LOG_INFO("%d led blink: %d of %d", led_index + 1, led_blink_counter, leds_blink_counts[led_index]);
            LOG_BACKEND_USB_PROCESS();
            led_blink_counter++;
            if (led_blink_counter > leds_blink_counts[led_index])
            {
                led_blink_counter = 1;
                led_index++;
                led_index %= ARRAY_SIZE(leds_blink_counts);
            }
            counter_ms = 0;
        }

        if (pwm_delay_ms >= pwm_percent_delay_ms)
        {
            // for first 500 ms pwm increases, for second one - decreases
            if (counter_ms < BLINK_DELAY_MS / 2)
                pwm_percentage++;
            else
                pwm_percentage--;
            pwm_delay_ms = 0;
        }

        pwm_delay_ms++;
        counter_ms++;
    }
}