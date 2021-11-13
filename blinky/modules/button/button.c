#include "button.h"

// Before init in_pin_handler method MUST be implemented.
void button_interrupt_init()
{
    nrfx_err_t err_code = nrfx_gpiote_init();
    APP_ERROR_CHECK(err_code);
    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BUTTON1, &in_config, in_pin_handler);
    nrfx_gpiote_in_event_enable(BUTTON1, true);
}