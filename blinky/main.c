/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

/**
 * @brief Function for application main entry.
 */

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


int main(void)
{
    led_t leds[] = {YELLOW, RED, GREEN, BLUE};
    int counts[] = {6, 6, 1, 3};

    for (size_t i = 0; i < ARRAY_SIZE(leds); i++)
    {
        init_led(leds[i]);
    }

    init_button(BUTTON1);

    int pos = 0;
    int counter = 1;
    int delcounter = 0;
    char stage = 0;
    while (true)
    {
        if(!is_button_pressed(BUTTON1))
            continue;

        if(delcounter > BLINK_DELAY_MS)
        {
            change_led_state(leds[pos]);
            stage = !stage;
            if(!stage)
                counter++;
            delcounter = 0;
            if (counter > counts[pos])
            {
                counter = 1;
                pos++;
                pos %= ARRAY_SIZE(counts);
            }
        }

        delcounter++;
        delay_ms(1);
    }
}
