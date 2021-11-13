#include "led.h"

led_t leds[] = {LED_YELLOW, LED_RED, LED_GREEN, LED_BLUE};
int leds_blink_counts[] = {6, 6, 1, 3};

volatile int led_index = 0;
volatile int led_blink_counter = 1;

bool led_check_for_change(int counter_ms)
{
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
        return true;
    }
    return false;
}

void led_init(led_t led)
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

led_t led_get_current()
{
    return leds[led_index];
}