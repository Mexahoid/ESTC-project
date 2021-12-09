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

#include "helpers.h"

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

#define USB
#ifdef USB
#include "usb.h"
#include "usb_commands.h"
#endif

/**
 * @brief Delegate for PWM methods.
 *
 * @param gpio                  GPIO for PWM
 * @param state                 State to change to
 */
void gpio_action(int gpio, int state)
{
    led_change_state_to(gpio, state);
}

int main(void)
{
    leds_init();
    color_rgb_t saved_rgb;
    flash_word_t data;

#ifdef USB
    usb_data_t usb_data;
    usb_data_t usb_data_old;
    usb_init(&usbc_process_command);
    usbc_init(&usb_data, &color_get_current_rgb, USB_BUFF_MESSAGE_SIZE);
#endif

#ifdef MAIN_LOG
#ifndef USB
    logs_init();
#endif

    int a = 0, b = 0, c = 0;
    int j = 0, k = 0, l = 0;
    int z = 0, x = 0, y = 0;
    int a1 = 0, a2 = 0, a3 = 0;

#endif

    if (!flash_init())
    {
        color_init(NULL);
    }
    else
    {
        flash_load_word(&data);

        saved_rgb.r = (int32_t)data.first_byte;
        saved_rgb.g = (int32_t)data.second_byte;
        saved_rgb.b = (int32_t)data.third_byte;
#ifdef MAIN_LOG
        a = saved_rgb.r;
        b = saved_rgb.g;
        c = saved_rgb.b;
#endif
        // Just in case
        saved_rgb.r = helper_clamp(saved_rgb.r, 0, 255);
        saved_rgb.g = helper_clamp(saved_rgb.g, 0, 255);
        saved_rgb.b = helper_clamp(saved_rgb.b, 0, 255);

        color_hsv_t hsv;
        color_convert_rgb_hsv(&saved_rgb, &hsv);

        color_init(&hsv);
#ifdef MAIN_LOG
        j = hsv.h;
        k = hsv.s;
        l = hsv.v;
        color_rgb_t test_rgb;
        color_get_current_rgb(&test_rgb);
        z = test_rgb.r;
        x = test_rgb.g;
        y = test_rgb.b;
        color_pwm_t test_rg2b;
        color_get_current_pwm_percentages(&test_rg2b);
        a1 = test_rg2b.r;
        a2 = test_rg2b.g;
        a3 = test_rg2b.b;

#endif
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

#ifdef USB
    usb_data.usb_color_command = USB_COM_RGB;
    usb_data.field1 = color.r;
    usb_data.field2 = color.g;
    usb_data.field3 = color.b;
    memcpy(&usb_data_old, &usb_data, sizeof(usb_data_t));
#endif

    char sflag = 0;

    while (true)
    {

#ifdef USB
        usb_process();
        if (memcmp(&usb_data_old, &usb_data, sizeof(usb_data_t)) != 0)
        {
#ifdef MAIN_LOG
        NRF_LOG_INFO("INITED: R: %d, G: %d, B: %d.", a, b, c);
        NRF_LOG_PROCESS();
        NRF_LOG_INFO("HSV: %d %d %d. GOT FROM COLOR: %d %d %d", j, k, l, z, x, y);
        NRF_LOG_PROCESS();
        NRF_LOG_INFO("PWMS: R: %d, G: %d, B: %d.", a1, a2, a3);
        NRF_LOG_PROCESS();
#endif
            memcpy(&usb_data_old, &usb_data, sizeof(usb_data_t));
            color_hsv_t hsv;

            switch (usb_data.usb_color_command)
            {
            case USB_COM_RGB:
                saved_rgb.r = usb_data.field1;
                saved_rgb.g = usb_data.field2;
                saved_rgb.b = usb_data.field3;
                color_set_rgb(&saved_rgb);
                break;

            case USB_COM_HSV:

                hsv.h = usb_data.field1;
                hsv.s = usb_data.field2;
                hsv.v = usb_data.field3;
                color_set_hsv(&hsv);
                break;
            default:
            break;
            }
            color_get_current_rgb(&saved_rgb);
            data.first_byte = (uint8_t)(saved_rgb.r);
            data.second_byte = (uint8_t)(saved_rgb.g);
            data.third_byte = (uint8_t)(saved_rgb.b);
            is_saved = true;
            flash_save_word(&data);
            color_get_current_pwm_percentages(&color);
        }
#endif


#ifdef MAIN_LOG
        logs_process();
#endif
        pwm_set_percentage(&pwm_context_led2_red, color.r);
        pwm_set_percentage(&pwm_context_led2_green, color.g);
        pwm_set_percentage(&pwm_context_led2_blue, color.b);

        pwm_modulate(&pwm_context_led1_green);
        pwm_modulate(&pwm_context_led2_red);
        pwm_modulate(&pwm_context_led2_green);
        pwm_modulate(&pwm_context_led2_blue);
        color_mode_t cm = color_get_mode();
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
                data.first_byte = (uint8_t)(saved_rgb.r);
                data.second_byte = (uint8_t)(saved_rgb.g);
                data.third_byte = (uint8_t)(saved_rgb.b);
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
                NRF_LOG_INFO("R: %d, G: %d, B: %d", color.r, color.g, color.b);
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