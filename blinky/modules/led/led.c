#include "led.h"

// Array with LED themselves.
static led_t leds[] = {LED_YELLOW, LED_RED, LED_GREEN, LED_BLUE};

// Inits LED as a GPIO.
static void led_init(led_t led)
{
    nrf_gpio_cfg_output(led);
    nrf_gpio_pin_write(led, 1);
}

void led_change_state_to(led_t led, led_state_t state)
{
    nrf_gpio_pin_write(led, state);
}

void leds_init()
{
    for (int i = 0; i < ARRAY_SIZE(leds); i++)
        led_init(leds[i]);
}