#include "led.h"

void led_init(led_t led)
{
    nrf_gpio_cfg_output(led);
    nrf_gpio_pin_write(led, 1);
}

void change_led_state_to(led_t led, led_state_t state)
{
    nrf_gpio_pin_write(led, state);
}