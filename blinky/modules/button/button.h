#ifndef MY_MODULE_BUTTON
#define MY_MODULE_BUTTON

#include "nrf_gpio.h"
#include "nrfx_gpiote.h"
#include "nrfx_systick.h"
#include "nrf_log.h"
#include "nrf_log_backend_usb.h"
#include "app_timer.h"

// Delay between clicks.
#define BUTTON_DOUBLECLICK_DELAY_MS 500

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
    NOP = 0,
    // When the button was doubleclicked.
    DOUBLECLICK = 1,
    // When the button is long pressed.
    LONGPRESS = 2
} button_state_t;


// Init interrupt for BUTTON1.
void button_interrupt_init();

// Checks and writes to is_automatic whether the button was doubleclicked.
button_state_t button_check_for_clicktype();

#endif