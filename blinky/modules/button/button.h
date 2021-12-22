#ifndef MY_MODULE_BUTTON
#define MY_MODULE_BUTTON

#include "nrf_gpio.h"
#include "nrfx_gpiote.h"
#include "nrfx_systick.h"
#include "app_timer.h"


// Delay between clicks.
#define BUTTON_DOUBLECLICK_DELAY_MS 500

// Delay for chatter mitigation.
#define BUTTON_CHATTER_DELAY_US 5000

// Delay with us in ms.
#define BUTTON_DELAY_US_IN_MS 1000

// Struct holding buttons pins.
typedef enum
{
    // Main button on the PCB.
    BUTTON1 = NRF_GPIO_PIN_MAP(1, 6),
} button_t;

// Struct holding button states.
typedef enum
{
    // When the button is not touched.
    BUTTON_STATE_NOP = 0,
    // When the button was doubleclicked.
    BUTTON_STATE_DOUBLECLICK = 1,
    // When the button is long pressed.
    BUTTON_STATE_LONGPRESS = 2
} button_state_t;


/**
 * @brief Init interrupt for BUTTON1.
 *
 */
void button_interrupt_init();

/**
 * @brief Checks and writes to is_automatic whether the button was doubleclicked.
 *
 * @return button_state_t
 */
button_state_t button_check_for_clicktype();

#endif