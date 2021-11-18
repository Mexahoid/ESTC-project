#include "button.h"

//#define BUTTON_LOG

// Current button press flag.
static volatile bool button_pressed_flag = false;
// Timestamp for the us-ms counter.
static nrfx_systick_state_t timestamp;
// Button clicks counter.
static volatile int clicks = 0;
// us counter.
static volatile int counter_us = 0;
// ms counter.
static volatile int counter_ms = 0;
// Is button long pressed.
static volatile bool long_press_present = false;

// Interrupt handler.
static void in_pin_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    UNUSED_VARIABLE(pin);
    UNUSED_VARIABLE(action);

    static nrfx_systick_state_t button_timestamp;

    button_pressed_flag = !button_pressed_flag;

    // Chatter mitigation
    if (!nrfx_systick_test(&button_timestamp, 5000))
        return;
    nrfx_systick_get(&button_timestamp);
    clicks++;
    //if (clicks > 4)
    //    clicks = 0;
#ifdef BUTTON_LOG
    NRF_LOG_INFO("Clicks: %d", clicks);
    LOG_BACKEND_USB_PROCESS();
#endif
}

void button_interrupt_init()
{
    nrfx_err_t err_code = nrfx_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    nrfx_gpiote_in_init(BUTTON1, &in_config, in_pin_handler);
    nrfx_gpiote_in_event_enable(BUTTON1, true);

    nrfx_systick_get(&timestamp);
}

button_state_t button_check_for_clicktype()
{
    if (nrfx_systick_test(&timestamp, 10))
    {
        counter_us += 10;
        if (counter_us >= 1000)
        {
            counter_ms++;
            counter_us = 0;
        }
        nrfx_systick_get(&timestamp);
    }

    bool delay_passed = counter_ms >= BUTTON_DOUBLECLICK_DELAY_MS;

    if (delay_passed)
    {
        counter_ms = 0;

        if (clicks == 1 && button_pressed_flag)
        {
            long_press_present = true;
#ifdef BUTTON_LOG
            NRF_LOG_INFO("Long press acquired");
            LOG_BACKEND_USB_PROCESS();
#endif
            return LONGPRESS;
        }

        if (clicks > 0)
        {
            clicks = 0;
            long_press_present = false;
#ifdef BUTTON_LOG
            NRF_LOG_INFO("No second clicks");
            LOG_BACKEND_USB_PROCESS();
#endif
            return NOP;
        }
    }
    else
    {
        if (long_press_present)
        {
            if (button_pressed_flag)
                return LONGPRESS;
            else
                long_press_present = false;
        }

        if (clicks > 3)
        {
            clicks = 0;
            long_press_present = false;
#ifdef BUTTON_LOG
            NRF_LOG_INFO("Double click acquired - changing mode");
            LOG_BACKEND_USB_PROCESS();
#endif
            return DOUBLECLICK;
        }
    }

    return NOP;
}