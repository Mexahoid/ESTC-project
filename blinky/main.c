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

#include <stdbool.h>
#include <stdint.h>
#include "boards.h"
#include "nordic_common.h"

#include "pwm.h"
// Frequency of PWM in kHz.
#define PWM_FREQUENCY 1000

#include "led.h"
// Microseconds delay for a one blink.
#define BLINK_DELAY_MS 1000

#include "button.h"
#include "color.h"

// Enables logging in main.
// #define MAIN_LOG
#ifdef MAIN_LOG
#include "nrf_log.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "log.h"
#endif


// Delegate for PWM methods.
void gpio_action(int gpio, int state_on)
{
    led_change_state_to(gpio, state_on);
}

int main(void)
{
    leds_init();
    color_init();

    pwm_ctx_t pwm_context_led1, pwm_context_red, pwm_context_green, pwm_context_blue;

    pwm_init(&pwm_context_led1, gpio_action, LED_ON, BLINK_DELAY_MS, PWM_FREQUENCY);
    pwm_init(&pwm_context_red, gpio_action, LED_ON, BLINK_DELAY_MS, PWM_FREQUENCY);
    pwm_init(&pwm_context_green, gpio_action, LED_ON, BLINK_DELAY_MS, PWM_FREQUENCY);
    pwm_init(&pwm_context_blue, gpio_action, LED_ON, BLINK_DELAY_MS, PWM_FREQUENCY);

    pwm_context_red.pwm_is_recalcable = false;
    pwm_context_green.pwm_is_recalcable = false;
    pwm_context_blue.pwm_is_recalcable = false;
    button_interrupt_init();

    color_pwm_t color;
    color_convert(&color);

#ifdef MAIN_LOG
    logs_init();
    color_pwm_t color_old;
    color_old.r = color.r;
    color_old.g = color.g;
    color_old.b = color.b;
#endif

    while (true)
    {
#ifdef MAIN_LOG
        logs_empty_action();
#endif

        pwm_modulate(&pwm_context_led1, LED1_GREEN);
        pwm_modulate(&pwm_context_red, LED2_RED);
        pwm_modulate(&pwm_context_green, LED2_GREEN);
        pwm_modulate(&pwm_context_blue, LED2_BLUE);
        color_mode_t cm = color_get_mode();
        switch (cm)
        {
        case CLR_OFF:
            pwm_context_led1.delay_total = BLINK_DELAY_MS;
            pwm_context_led1.pwm_is_recalcable = false;
            pwm_set_percentage(&pwm_context_led1, 0);
            break;
        case CLR_HUE:
            pwm_context_led1.delay_total = BLINK_DELAY_MS;
            pwm_context_led1.pwm_is_recalcable = true;
            pwm_percentage_recalc(&pwm_context_led1);
            break;
        case CLR_SAT:
            pwm_context_led1.delay_total = BLINK_DELAY_MS / 2;
            pwm_context_led1.pwm_is_recalcable = true;
            pwm_percentage_recalc(&pwm_context_led1);
            break;
        case CLR_BRI:
            pwm_context_led1.delay_total = BLINK_DELAY_MS;
            pwm_context_led1.pwm_is_recalcable = false;
            pwm_set_percentage(&pwm_context_led1, 100);
            break;
        }

        button_state_t button_state = button_check_for_clicktype();

        pwm_set_percentage(&pwm_context_red, color.r);
        pwm_set_percentage(&pwm_context_green, color.g);
        pwm_set_percentage(&pwm_context_blue, color.b);

        if (button_state == BTN_LONGPRESS && cm != CLR_OFF)
        {
            color_increase_mode_value();
            color_convert(&color);

#ifdef MAIN_LOG
            if (color_old.r != color.r || color_old.g != color.g || color_old.b != color.b)
            {
                NRF_LOG_INFO("R: %d, G: %d, B: %d", color.r, color.g, color.b);
                LOG_BACKEND_USB_PROCESS();
                color_old.r = color.r;
                color_old.g = color.g;
                color_old.b = color.b;
            }
#endif
        }

        if (button_state == BTN_DOUBLECLICK)
        {
            color_change_mode();
#ifdef MAIN_LOG
            NRF_LOG_INFO("Color mode changed: %d", color_get_mode());
            LOG_BACKEND_USB_PROCESS();
#endif
        }
    }
}