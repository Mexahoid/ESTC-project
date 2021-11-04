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

#define BLINK_DELAY 500

typedef enum LED
{
    Yellow = 6,
    Red = 8,
    Green = 41,
    Blue = 12,
} LED;

typedef enum BUTTON
{
    Button1 = 38,
} BUTTON;

void initButton(BUTTON btn)
{
    nrf_gpio_cfg_input(btn, GPIO_PIN_CNF_PULL_Pullup);
}

int readButtonState(BUTTON btn)
{
    return !nrf_gpio_pin_read(btn);
}

void initLed(LED led)
{
    nrf_gpio_cfg_output(led);
    nrf_gpio_pin_write(led, 1);
}

void changeLedState(LED led)
{
    nrf_gpio_pin_toggle(led);
}


void blink(LED led)
{
    for (char i = 0; i < 2; i++)
    {
        changeLedState(led);
        nrf_delay_ms(BLINK_DELAY);
    }
}

int main(void)
{
    LED leds[] = {Yellow, Red, Green, Blue};
    int counts[] = {6, 6, 1, 3};

    for (size_t i = 0; i < sizeof(leds)/sizeof(LED); i++)
    {
        initLed(leds[i]);
    }

    initButton(Button1);

    int pos = 0;
    int counter = 1;
    int delcounter = 0;
    char stage = 0;
    while (true)
    {
        if(!readButtonState(Button1))
            continue;

        if(delcounter > BLINK_DELAY)
        {
            changeLedState(leds[pos]);
            stage = !stage;
            if(!stage)
                counter++;
            delcounter = 0;
            if (counter > counts[pos])
            {
                counter = 1;
                pos++;
                pos %= sizeof(counts)/sizeof(int);
            }
        }

        delcounter++;
        nrf_delay_ms(1);
    }
}
