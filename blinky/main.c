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

#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#include "nrf_log.h"
#include "nrf_log_backend_usb.h"

#include "log.h"
#include "led.h"
#include "button.h"
#include "pwm.h"

// Delegate for PWM methods.
void gpio_action(int gpio, int state_on)
{
    led_change_state_to(gpio, state_on);
}

int main(void)
{
    logs_init();
    leds_init();
    pwm_init(gpio_action, LED_ON, BLINK_DELAY_MS);
    button_interrupt_init();

    bool is_automatic = false;
    while (true)
    {
        logs_empty_action();

        pwm_modulate(led_get_current());

        bool dc_present = button_check_for_doubleclick();

        if (dc_present)
            is_automatic = !is_automatic;

        if (!is_automatic)
            continue;

        if (pwm_is_delay_passed())
            led_change_for_next();

        pwm_percentage_recalc();
    }
}