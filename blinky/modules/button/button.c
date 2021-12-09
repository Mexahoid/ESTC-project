#include "button.h"

// Current button press flag.
static volatile bool is_button_pressed = false;
// Timestamp for the us-ms counter.
static nrfx_systick_state_t timestamp;
// Button clicks counter.
static volatile int clicks = 0;
// ms counter.
static volatile int counter_ms = 0;
// Is button long pressed.
static volatile bool is_long_pressed = false;

/**
 * @brief Interrupt handler.
 *
 * @param pin                   Button pin
 * @param action                Action for an interrupt
 */
static void in_pin_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    UNUSED_VARIABLE(pin);
    UNUSED_VARIABLE(action);

    static nrfx_systick_state_t button_timestamp;

    is_button_pressed = !is_button_pressed;

    // Chatter mitigation
    if (!nrfx_systick_test(&button_timestamp, BUTTON_CHATTER_DELAY_US))
        return;
    nrfx_systick_get(&button_timestamp);
    clicks++;
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
    if (!nrfx_systick_test(&timestamp, BUTTON_DELAY_US_IN_MS))
        return BUTTON_STATE_NOP;
    counter_ms++;
    nrfx_systick_get(&timestamp);

    bool delay_passed = counter_ms >= BUTTON_DOUBLECLICK_DELAY_MS;

    if (delay_passed)
    {
        counter_ms = 0;

        if (clicks == 1 && is_button_pressed)
        {
            is_long_pressed = true;
#ifdef BUTTON_LOG
            NRF_LOG_INFO("Long press acquired");
            LOG_BACKEND_USB_PROCESS();
#endif
            return BUTTON_STATE_LONGPRESS;
        }

        if (clicks > 0)
        {
            clicks = 0;
            is_long_pressed = false;
#ifdef BUTTON_LOG
            NRF_LOG_INFO("No second clicks");
            LOG_BACKEND_USB_PROCESS();
#endif
            return BUTTON_STATE_NOP;
        }
    }
    else
    {
        if (is_long_pressed)
        {
            if (is_button_pressed)
                return BUTTON_STATE_LONGPRESS;
            else
                is_long_pressed = false;
        }

        if (clicks > 3)
        {
            clicks = 0;
            is_long_pressed = false;
#ifdef BUTTON_LOG
            NRF_LOG_INFO("Double click acquired - changing mode");
            LOG_BACKEND_USB_PROCESS();
#endif
            return BUTTON_STATE_DOUBLECLICK;
        }
    }

    return BUTTON_STATE_NOP;
}