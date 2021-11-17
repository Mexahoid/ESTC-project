#include "pwm.h"

// Inner module counter [0 - PWM_COUNTER_MAX].
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
// Total time that a GPIO suffers from PWM.
int delay_total = 0;
// Time that PWM should be ON.
volatile int pwm_on_time = 0;
// us timestamp for tick updates.
nrfx_systick_state_t timestamp_pwm_us;
// ms timestamp for tick updates.
nrfx_systick_state_t timestamp_pwm_ms;

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

    if (current_ms % pwm_gpio_percent_ms != 0)
        return;

    pwm_on_time = pwm_duty_delay_us * pwm_percentage / 100;

    if (current_ms < delay_total / 2 && pwm_percentage < 100)
        pwm_percentage++;
    else
        pwm_percentage--;
}

bool pwm_is_delay_passed()
{
    if (current_ms >= delay_total)
    {
        current_ms = 0;
        pwm_percentage = 0;
        return true;
    }
    return false;
}

void pwm_init(void (*action)(int, int), int state_on, int total_time)
{
    nrfx_systick_init();
    nrfx_systick_get(&timestamp_pwm_us);
    nrfx_systick_get(&timestamp_pwm_ms);
    pwm_state_on = state_on;
    pwm_action = action;
    delay_total = total_time;
    pwm_gpio_percent_ms = total_time / 100 / 2;
}

bool pwm_is_ms_passed()
{
    if (!nrfx_systick_test(&timestamp_pwm_ms, PWM_MAX_MS_DELAY))
        return false;
    nrfx_systick_get(&timestamp_pwm_ms);
    current_ms++;
    return true;
}

void pwm_tick_update()
{
    if (!nrfx_systick_test(&timestamp_pwm_us, pwm_gpio_percent_ms))
        return;
    nrfx_systick_get(&timestamp_pwm_us);
    pwm_counter += pwm_gpio_percent_ms;
    if (pwm_counter >= PWM_COUNTER_MAX)
        pwm_counter = 0;
}