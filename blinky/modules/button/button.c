#include "button.h"

// Current button press flag.
volatile bool button_pressed_flag = false;
// Previous button state for comparing.
volatile bool button_prev_state = false;
// Timestamp of first button click.
nrfx_systick_state_t button_timestamp;
// Counter of button clicks.
volatile int button_press_counter = 0;

void button_interrupt_init()
{
    nrfx_err_t err_code = nrfx_gpiote_init();
    APP_ERROR_CHECK(err_code);
    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BUTTON1, &in_config, button_in_pin_handler);
    nrfx_gpiote_in_event_enable(BUTTON1, true);

    NRF_LOG_INFO("Button interrupt initialized");
    LOG_BACKEND_USB_PROCESS();
}

void button_in_pin_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    UNUSED_VARIABLE(pin);
    UNUSED_VARIABLE(action);

    button_pressed_flag = !button_pressed_flag;
}

bool button_check_for_doubleclick()
{
    bool flag = false;
    if (button_prev_state != button_pressed_flag)
    {
        button_press_counter++;
        nrfx_systick_get(&button_timestamp);

        NRF_LOG_INFO("Click found, presses: %d, time: %d", button_press_counter, button_timestamp.time);
        LOG_BACKEND_USB_PROCESS();
    }

    if (button_press_counter > 0)
    {
        if (nrfx_systick_test(&button_timestamp, BUTTON_DOUBLECLICK_DELAY_MS * 1000))
        {
            button_press_counter = 0;
            nrfx_systick_get(&button_timestamp);
            NRF_LOG_INFO("There were no second clicks, time: %d", button_timestamp.time);
            LOG_BACKEND_USB_PROCESS();
        }
        if (button_press_counter > 1)
        {
            NRF_LOG_INFO("Changing mode");
            LOG_BACKEND_USB_PROCESS();
            flag = true;
            button_press_counter = 0;
        }
    }

    button_prev_state = button_pressed_flag;
    return flag;
}