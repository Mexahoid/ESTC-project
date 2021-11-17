#include "button.h"

// Current button press flag.
volatile bool button_pressed_flag = false;
// Timestamp of first button click.
nrfx_systick_state_t button_timestamp;
// Counter of button clicks.
volatile int button_press_counter = 0;

volatile int clicks = 0;
volatile bool temp = false;

void button_in_pin_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    UNUSED_VARIABLE(pin);
    UNUSED_VARIABLE(action);

    button_pressed_flag = !button_pressed_flag;
    clicks++;
}

void button_interrupt_init()
{
    nrfx_err_t err_code = nrfx_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    nrfx_gpiote_in_init(BUTTON1, &in_config, button_in_pin_handler);
    nrfx_gpiote_in_event_enable(BUTTON1, true);

    NRF_LOG_INFO("Button interrupt initialized");
    LOG_BACKEND_USB_PROCESS();
}

int button_check_for_clicktype()
{
    int code = 0;

    if (clicks < 2)
        return code;

    if (clicks > 1 && nrfx_systick_test(&button_timestamp, BUTTON_DOUBLECLICK_DELAY_MS * 1000))
    {
        clicks = 0;
        temp = false;
        NRF_LOG_INFO("No second clicks");
        LOG_BACKEND_USB_PROCESS();
        return code;
    }

    if (clicks > 3 && !nrfx_systick_test(&button_timestamp, BUTTON_DOUBLECLICK_DELAY_MS * 1000))
    {
        code = 1;
        clicks = 0;
        temp = false;
        NRF_LOG_INFO("Double click acquired - changing mode");
        LOG_BACKEND_USB_PROCESS();
        return code;
    }

    if (clicks > 1)
    {
        if (!temp)
        {
            nrfx_systick_get(&button_timestamp);
            temp = true;
        }
    }

    return code;
}