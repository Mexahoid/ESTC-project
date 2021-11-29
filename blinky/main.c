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

#include "flash.h"

// Enables logging in main.
// #define MAIN_LOG
#ifdef MAIN_LOG
#include "nrf_log.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "log.h"
#include "nrf_delay.h"
#endif

// Delegate for PWM methods.
void gpio_action(int gpio, int state_on)
{
    led_change_state_to(gpio, state_on);
}

int main(void)
{
    leds_init();
    color_rgb_t saved_rgb;
    flash_word_t data;

#ifdef MAIN_LOG
    logs_init();
    NRF_LOG_INFO("Init led init");
    NRF_LOG_PROCESS();
    //nrf_delay_ms(3000);
    bool flash_flag = false;
    int addr = -1;
#endif

    if (!flash_init())
    {
#ifdef MAIN_LOG
        NRF_LOG_INFO("Not found data, init");
        NRF_LOG_PROCESS();
        flash_flag = false;
#endif

        color_init(0);
    }
    else
    {
        flash_load_word(&data);
#ifdef MAIN_LOG
        flash_flag = true;
        NRF_LOG_INFO("R: %d, G: %d, B: %d", data.first_byte, data.second_byte, data.third_byte);
        NRF_LOG_PROCESS();
#endif
        saved_rgb.r = data.first_byte;
        saved_rgb.g = data.second_byte;
        saved_rgb.b = data.third_byte;
        color_init(&saved_rgb);
    }

    color_pwm_t color;
    color_get_current_pwm_percentages(&color);

    pwm_ctx_t pwm_context_led2_blue;
    pwm_init(&pwm_context_led2_blue, gpio_action, LED_ON, 0, PWM_FREQUENCY, LED2_BLUE);

    pwm_ctx_t pwm_context_led2_red;
    pwm_init(&pwm_context_led2_red, gpio_action, LED_ON, 0, PWM_FREQUENCY, LED2_RED);

    pwm_ctx_t pwm_context_led2_green;
    pwm_init(&pwm_context_led2_green, gpio_action, LED_ON, 0, PWM_FREQUENCY, LED2_GREEN);

    pwm_ctx_t pwm_context_led1_green;
    pwm_init(&pwm_context_led1_green, gpio_action, LED_ON, BLINK_DELAY_MS, PWM_FREQUENCY, LED1_GREEN);

    button_interrupt_init();

    bool is_saved = true;

#ifdef MAIN_LOG
    color_pwm_t color_old;
    color_old.r = color.r;
    color_old.g = color.g;
    color_old.b = color.b;
#endif

    char sflag = 0;

    while (true)
    {
#ifdef MAIN_LOG
        logs_empty_action();
#endif
#ifdef MAIN_LOG
        NRF_LOG_INFO("R: %d, G: %d, B: %d, flash_flag: %d, addr: %d", color.r, color.g, color.b, flash_flag, addr);
        NRF_LOG_PROCESS();
#endif
        pwm_set_percentage(&pwm_context_led2_red, color.r);
        pwm_set_percentage(&pwm_context_led2_green, color.g);
        pwm_set_percentage(&pwm_context_led2_blue, color.b);

        pwm_modulate(&pwm_context_led1_green);
        pwm_modulate(&pwm_context_led2_red);
        pwm_modulate(&pwm_context_led2_green);
        pwm_modulate(&pwm_context_led2_blue);
        color_mode_t cm = color_get_mode();
        //continue;
        switch (cm)
        {
        case COLOR_MODE_OFF:
            if ((sflag & 0b0001) == 0)
            {
                pwm_context_led1_green.delay_total = 0;
                pwm_set_percentage(&pwm_context_led1_green, 0);
                sflag &= 0b0001;
            }

            if (!is_saved)
            {
                color_get_current_rgb(&saved_rgb);
                data.first_byte = (unsigned char)(saved_rgb.r);
                data.second_byte = (unsigned char)(saved_rgb.g);
                data.third_byte = (unsigned char)(saved_rgb.b);
                is_saved = true;
#ifdef MAIN_LOG
                NRF_LOG_INFO("Saving: R: %d, G: %d, B: %d, is_saved: %d", data.first_byte, data.second_byte, data.third_byte, is_saved);
                NRF_LOG_PROCESS();
#endif
                flash_save_word(&data);
            }

            break;
        case COLOR_MODE_HUE:
            if ((sflag & 0b0010) == 0)
            {
                pwm_context_led1_green.delay_total = BLINK_DELAY_MS;
                sflag &= 0b0010;
            }
            break;
        case COLOR_MODE_SAT:
            if ((sflag & 0b0100) == 0)
            {
                pwm_context_led1_green.delay_total = BLINK_DELAY_MS / 2;
                sflag &= 0b0100;
            }
            break;
        case COLOR_MODE_BRI:
            if ((sflag & 0b1000) == 0)
            {
                pwm_context_led1_green.delay_total = 0;
                pwm_set_percentage(&pwm_context_led1_green, 100);
                sflag &= 0b1000;
            }
            break;
        }
        pwm_percentage_recalc(&pwm_context_led1_green);

        button_state_t button_state = button_check_for_clicktype();

        if (button_state == BUTTON_STATE_LONGPRESS && cm != COLOR_MODE_OFF)
        {
            color_increase_mode_value();
            color_get_current_pwm_percentages(&color);
            if (is_saved)
                is_saved = false;

#ifdef MAIN_LOG
            if (color_old.r != color.r || color_old.g != color.g || color_old.b != color.b)
            {
                NRF_LOG_INFO("R: %d, G: %d, B: %d, is_saved: %d", color.r, color.g, color.b, is_saved);
                NRF_LOG_PROCESS();
                color_old.r = color.r;
                color_old.g = color.g;
                color_old.b = color.b;
            }
#endif
        }

        if (button_state == BUTTON_STATE_DOUBLECLICK)
        {
            color_change_mode();
#ifdef MAIN_LOG
            NRF_LOG_INFO("Color mode changed: %d", color_get_mode());
            NRF_LOG_PROCESS();
#endif
        }
    }
}