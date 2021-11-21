#ifndef MY_MODULE_PWM
#define MY_MODULE_PWM

#include "nrfx_systick.h"

// Max inner counter counts before nullying.
#define PWM_COUNTER_MAX 1000
// Max amount of ms in s (1000 lol).
#define PWM_MAX_MS_DELAY 1000
// Inner counter us delay.
#define PWM_TICK_DELAY_US 20

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
    // Is current context GPIO automatically recalcable.
    bool pwm_is_recalcable;
} pwm_ctx_t;


// Modulates GPIO with PWM.
void pwm_modulate(pwm_ctx_t* context);

// Looks every PWM ms for a PWM percentage recalc.
void pwm_percentage_recalc(pwm_ctx_t* context);

// Initializes PWM module.
void pwm_init(pwm_ctx_t* context, void (*action)(int, int), int state_on, int on_time, int frequency, int gpio);

// Returns true if PWM total delay has passed.
bool pwm_is_delay_passed(pwm_ctx_t* context);

// Sets PWM percentage.
void pwm_set_percentage(pwm_ctx_t* context, int percentage);

#endif