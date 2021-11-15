#ifndef MY_MODULE_LED
#define MY_MODULE_LED

#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_backend_usb.h"

// Microseconds delay for a one blink.
#define BLINK_DELAY_MS 1000

// Struct that holds LED pin codes.
typedef enum
{
    LED_YELLOW = NRF_GPIO_PIN_MAP(0, 6),
    LED_RED = NRF_GPIO_PIN_MAP(0, 8),
    LED_GREEN = NRF_GPIO_PIN_MAP(1, 9),
    LED_BLUE = NRF_GPIO_PIN_MAP(0, 12),
} led_t;

// Struct that holds LED states.
typedef enum
{
    LED_ON = 0,
    LED_OFF = 1
} led_state_t;

// Inits LED as a GPIO.
void led_init(led_t led);

// Changes LED state to ON or OFF.
void led_change_state_to(led_t led, led_state_t state);

// Inits all LEDs.
void leds_init();

// Returns current working LED.
led_t led_get_current();

// Sets next LED as an active one.
void led_change_for_next();

#endif