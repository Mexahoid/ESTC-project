#include "button.h"

bool st_button_pressed_flag = false;
bool s_prev_button_state = false;
nrfx_systick_state_t timestamp_button;
int button_press_counter = 0;

// Adds interrupt on button lotohi click.
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

    st_button_pressed_flag = !st_button_pressed_flag;
}

void button_check_for_doubleclick(bool *is_automatic)
{
    if (s_prev_button_state != st_button_pressed_flag)
    {
        button_press_counter++;
        nrfx_systick_get(&timestamp_button);

        NRF_LOG_INFO("Click found, presses: %d, time: %d", button_press_counter, timestamp_button.time);
        LOG_BACKEND_USB_PROCESS();
    }

    if (button_press_counter > 0)
    {
        if (nrfx_systick_test(&timestamp_button, BUTTON_DOUBLECLICK_DELAY_MS * 1000))
        {
            button_press_counter = 0;
            nrfx_systick_get(&timestamp_button);
            NRF_LOG_INFO("There were no second clicks, time: %d", timestamp_button.time);
            LOG_BACKEND_USB_PROCESS();
        }
        if (button_press_counter > 1)
        {
            NRF_LOG_INFO("Changing mode");
            LOG_BACKEND_USB_PROCESS();
            *is_automatic = !*is_automatic;
            button_press_counter = 0;
        }
    }

    s_prev_button_state = st_button_pressed_flag;
}