#ifndef MY_MODULE_BUTTON
#define MY_MODULE_BUTTON

#include "nrf_gpio.h"
#include "nrfx_gpiote.h"

typedef enum
{
    BUTTON1 = NRF_GPIO_PIN_MAP(1, 6),
} button_t;

void button_interrupt_init();

#endif