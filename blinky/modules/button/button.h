#ifndef MY_MODULE_BUTTON
#define MY_MODULE_BUTTON

#include "nrf_gpio.h"
#include "nrfx_gpiote.h"
#include "nrfx_systick.h"
#include "nrf_log.h"
#include "nrf_log_backend_usb.h"

// Delay between clicks. If less - can't doubleclick, if more - inadequate behaviour.
#define BUTTON_DOUBLECLICK_DELAY_MS 250

// Struct holding buttons pins.
typedef enum
{
    // Main button on the PCB.
    BUTTON1 = NRF_GPIO_PIN_MAP(1, 6),
} button_t;

// Init interrupt for BUTTON1.
void button_interrupt_init();

// Checks and writes to is_automatic whether the button was doubleclicked.
void button_check_for_doubleclick(bool *is_automatic);

// Interrupt handler.
void button_in_pin_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

#endif