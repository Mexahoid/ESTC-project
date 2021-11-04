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

#define LED_DELAY 200
#define BLINK_DELAY 500

typedef enum
{
    Yellow = 6,
    Red = 8,
    Green = 5,
    Blue = 1,
} led;


void changeLedState(int led, int state)
{
    switch (led)
    {
    case 1:
        break;

    default:
        break;
    }
}


void blink(int led, int count)
{
    for (size_t i = 0; i < count * 2; i++)
    {
        bsp_board_led_invert(led);
        nrf_delay_ms(LED_DELAY);
    }
    nrf_delay_ms(BLINK_DELAY);
}

int main(void)
{
    nrf_gpio_cfg_output(6);
    nrf_gpio_cfg_output(8);
    nrf_gpio_cfg_output(41);
    nrf_gpio_cfg_output(12);


    while (true)
    {
        nrf_gpio_pin_write(6, 0);
        nrf_delay_ms(1000);
        nrf_gpio_pin_write(6, 1);
        nrf_delay_ms(1000);
        nrf_gpio_pin_write(8, 0);
        nrf_delay_ms(1000);
        nrf_gpio_pin_write(8, 1);
        nrf_delay_ms(1000);
        nrf_gpio_pin_write(41, 0);
        nrf_delay_ms(1000);
        nrf_gpio_pin_write(41, 1);
        nrf_delay_ms(1000);
        nrf_gpio_pin_write(12, 0);
        nrf_delay_ms(1000);
        nrf_gpio_pin_write(12, 1);
        nrf_delay_ms(1000);
    }
}
