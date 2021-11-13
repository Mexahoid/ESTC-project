#include "pwm.h"


// Inner module counter [0 - 1000].
volatile int pwm_counter = 0;
// PWM duty percentage.
volatile int pwm_percentage = 0;
// Amount (in us) for a cycle.
int pwm_duty_delay_us = 1000000 / PWM_FREQUENCY;
// Delegate for PWM modulation.
void (*pwm_action)(int, int);
// Default ON state for a GPIO, goes from init.
volatile int pwm_state_on;
// ms delay for a PWM percent.
volatile int pwm_gpio_percent_ms;
// Current ms for inner operations.
volatile int current_ms = 0;
// Time that PWM should be ON.
volatile int pwm_on_time = 0;
// us timestamp for tick updates.
nrfx_systick_state_t timestamp_pwm_us;


void pwm_modulate(int gpio)
{
    if (pwm_counter < pwm_on_time)
        pwm_action(gpio, pwm_state_on);
    else
        pwm_action(gpio, !pwm_state_on);
}

void pwm_percentage_recalc()
{
    if (!pwm_is_ms_passed())
        return;

    current_ms++;

    pwm_on_time = pwm_duty_delay_us * pwm_percentage / 100;
    if (current_ms % pwm_gpio_percent_ms != 0)
        return;

    if (current_ms < pwm_duty_delay_us / 2 && pwm_percentage < 100)
        pwm_percentage++;
    else
        pwm_percentage--;
    if (current_ms >= PWM_MAX_MS_DELAY)
    {
        current_ms = 0;
        pwm_percentage = 0;
    }
}

bool pwm_is_ms_passed()
{
    // Somehow only this makes it really work as a us-ms-s
    return pwm_counter % (PWM_COUNTER_MAX / 6) == 0;
}

void pwm_init(void (*action)(int, int), int state_on, int on_time)
{
    nrfx_systick_init();
    nrfx_systick_get(&timestamp_pwm_us);
    pwm_state_on = state_on;
    pwm_action = action;
    pwm_gpio_percent_ms = pwm_duty_delay_us / 100 / 2;
}

void pwm_tick_update()
{
    if (!nrfx_systick_test(&timestamp_pwm_us, PWM_US_DELAY))
        return;

    pwm_counter++;

    if (pwm_counter > PWM_COUNTER_MAX)
        pwm_counter = 0;
}