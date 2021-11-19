#ifndef MY_MODULE_LED
#define MY_MODULE_LED

#include "nrf_gpio.h"

// Enum that holds LED pin codes.
typedef enum
{
    LED_YELLOW = NRF_GPIO_PIN_MAP(0, 6),
    LED_RED = NRF_GPIO_PIN_MAP(0, 8),
    LED_GREEN = NRF_GPIO_PIN_MAP(1, 9),
    LED_BLUE = NRF_GPIO_PIN_MAP(0, 12),
} led_t;

// Enum that holds LED states.
typedef enum
{
    LED_ON = 0,
    LED_OFF = 1
} led_state_t;

// Changes LED state to ON or OFF.
void led_change_state_to(led_t led, led_state_t state);

// Inits all LEDs.
void leds_init();


#endif