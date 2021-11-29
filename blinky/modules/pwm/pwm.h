#ifndef MY_MODULE_PWM
#define MY_MODULE_PWM

#include "nrfx_systick.h"
#include <string.h>

// PWM context struct.
typedef struct
{
    int pwm_gpio;
    // Inner counter [0 - PWM_COUNTER_MAX].
    volatile int pwm_counter;
    // PWM duty percentage.
    volatile int pwm_percentage;
    // Amount (in us) for a cycle.
    int pwm_duty_delay_us;
    // Delegate for PWM modulation.
    void (*pwm_action)(int, int);
    // Default ON state for a GPIO, goes from init.
    volatile int pwm_state_on;
    // ms delay for a PWM percent.
    volatile int pwm_gpio_percent_ms;
    // Current ms for inner operations.
    volatile int current_ms;
    // Total time that a GPIO suffers from PWM.
    int delay_total;
    // Time that PWM should be ON.
    volatile int pwm_on_time;
    // us timestamp for tick updates.
    nrfx_systick_state_t pwm_timestamp_us;
    // ms timestamp for tick updates.
    nrfx_systick_state_t pwm_timestamp_ms;
} pwm_ctx_t;


// Modulates GPIO with PWM.
void pwm_modulate(pwm_ctx_t* const context);

// Looks every PWM ms for a PWM percentage recalc.
void pwm_percentage_recalc(pwm_ctx_t* const context);

// Initializes PWM module.
void pwm_init(pwm_ctx_t* const context, void (*action)(int, int), int state_on, int on_time, int frequency, int gpio);

// Sets PWM percentage.
void pwm_set_percentage(pwm_ctx_t* const context, int percentage);

#endif