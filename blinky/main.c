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
#include "nrfx_gpiote.h"

#include "led.h"

#define PWM_FREQUENCY 1000

#define BUTTON_DOUBLECLICK_DELAY_MS 250

typedef enum
{
    BUTTON1 = NRF_GPIO_PIN_MAP(1, 6),
} button_t;

void delay_us(int amount)
{
    nrf_delay_us(amount);
}

// Modulates PWM in relation to timestamp. Checks whether the difference between timestamp and present tick is more than pwm_delay_passed_us and ONs or OFFs LED.
void pwm_modulate_related(int pwm_delay_passed_us, led_t led, nrfx_systick_state_t *timestamp)
{
    if (!nrfx_systick_test(timestamp, pwm_delay_passed_us))
        change_led_state_to(led, LED_ON);
    else
        change_led_state_to(led, LED_OFF);
}

// Modulates LED as PWM on pwm_duty_delay_us for a certain pwm_percentage.
void pwm_modulate(int pwm_percentage, int pwm_duty_delay_us, led_t led)
{
    nrfx_systick_state_t timestamp_pwm;
    nrfx_systick_get(&timestamp_pwm);
    if (pwm_percentage <= 100 && pwm_percentage >= 0)
    {
        int frac = pwm_duty_delay_us / 100;
        nrfx_systick_get(&timestamp_pwm);
        for (int i = 0; i < pwm_duty_delay_us; i += frac)
        {
            pwm_modulate_related(pwm_percentage * frac, led, &timestamp_pwm);
            delay_us(frac);
        }
    }
}

static bool st_button_pressed_flag = false;

void in_pin_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    UNUSED_VARIABLE(pin);
    UNUSED_VARIABLE(action);

    st_button_pressed_flag = !st_button_pressed_flag;
}

int main(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    NRF_LOG_INFO("Starting up the test project with USB logging");
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    led_t leds[] = {LED_YELLOW, LED_RED, LED_GREEN, LED_BLUE};
    int leds_blink_counts[] = {6, 6, 1, 3};

    for (int i = 0; i < ARRAY_SIZE(leds); i++)
        init_led(leds[i]);

    nrfx_systick_init();
    nrfx_systick_state_t timestamp_button;

    nrfx_err_t err_code = nrfx_gpiote_init();
    APP_ERROR_CHECK(err_code);
    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BUTTON1, &in_config, in_pin_handler);
    nrfx_gpiote_in_event_enable(BUTTON1, true);

    int led_index = 0;
    int led_blink_counter = 1;

    int pwm_percentage = 0;
    // X ms for 1%, 1000 ms -> 500 ms for 0-100% -> 5ms for 1% pwm duty
    int pwm_percent_delay_ms = BLINK_DELAY_MS / 100 / 2;
    int pwm_delay_ms = 0;

    int counter_ms = 0;

    int pwm_duty_delay_us2 = 1000000 / PWM_FREQUENCY;

    bool is_automatic = false;
    bool prev_button_state = st_button_pressed_flag;

    int button_press_counter = -1;
    int button_delay = BUTTON_DOUBLECLICK_DELAY_MS * 1000;

    while (true)
    {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();

        pwm_modulate(pwm_percentage, pwm_duty_delay_us2, leds[led_index]);

        if (prev_button_state != st_button_pressed_flag)
        {
            button_press_counter++;
            nrfx_systick_get(&timestamp_button);
            NRF_LOG_INFO("Click found, presses: %d, time: %d", button_press_counter, timestamp_button.time);
            LOG_BACKEND_USB_PROCESS();
        }

        if (button_press_counter > 0)
        {
            if (nrfx_systick_test(&timestamp_button, button_delay))
            {
                button_press_counter = 0;
                nrfx_systick_get(&timestamp_button);
                NRF_LOG_INFO("There were no second clicks, time: %d", timestamp_button.time);
                LOG_BACKEND_USB_PROCESS();
            }
            if (button_press_counter > 1)
            {
                NRF_LOG_INFO("Changing mode");
                LOG_BACKEND_USB_PROCESS();
                is_automatic = !is_automatic;
                button_press_counter = 0;
            }
        }

        prev_button_state = st_button_pressed_flag;

        if (!is_automatic)
            continue;

        if (counter_ms >= BLINK_DELAY_MS)
        {
            NRF_LOG_INFO("%d led blinked: %d of %d", led_index + 1, led_blink_counter, leds_blink_counts[led_index]);
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
            if (counter_ms <= BLINK_DELAY_MS / 2 && pwm_percentage < 100)
                pwm_percentage++;
            else
                pwm_percentage--;
            pwm_delay_ms = 0;
        }

        pwm_delay_ms++;
        counter_ms++;
    }
}