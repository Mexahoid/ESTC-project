#include "pwm.h"

// Was systic initialized or not.
bool is_systick_init = false;

// Updates inner counter every PWM percent delay.
static void tick_update(pwm_ctx_t* context)
{
    if (!nrfx_systick_test(&(context->timestamp_pwm_us), context->pwm_gpio_percent_ms))
        return;
    nrfx_systick_get(&(context->timestamp_pwm_us));
    context->pwm_counter += context->pwm_gpio_percent_ms;
    if (context->pwm_counter >= PWM_COUNTER_MAX)
        context->pwm_counter = 0;
}

// Returns true if at least ms has passed.
static bool is_ms_passed(pwm_ctx_t* context)
{
    if (!nrfx_systick_test(&(context->timestamp_pwm_ms), PWM_MAX_MS_DELAY))
        return false;
    nrfx_systick_get(&(context->timestamp_pwm_ms));
    context->current_ms++;
    return true;
}


void pwm_modulate(pwm_ctx_t* context, int gpio)
{
    tick_update(context);
    if (context->pwm_counter < context->pwm_on_time)
        context->pwm_action(gpio, context->pwm_state_on);
    else
        context->pwm_action(gpio, !context->pwm_state_on);
}

void pwm_percentage_recalc(pwm_ctx_t* context)
{
    if (!is_ms_passed(context))
        return;

    if (context->current_ms % context->pwm_gpio_percent_ms != 0)
        return;

    context->pwm_on_time = context->pwm_duty_delay_us * context->pwm_percentage / 100;

    if (context->current_ms < context->delay_total / 2 && context->pwm_percentage < 100)
        context->pwm_percentage++;
    else
        context->pwm_percentage--;
}

bool pwm_is_delay_passed(pwm_ctx_t* context)
{
    if (context->current_ms >= context->delay_total)
    {
        context->current_ms = 0;
        context->pwm_percentage = 0;
        return true;
    }
    return false;
}

void pwm_init(pwm_ctx_t* context, void (*action)(int, int), int state_on, int total_time, int frequency)
{
    if (!is_systick_init)
    {
        nrfx_systick_init();
        is_systick_init = true;
    }

    nrfx_systick_get(&(context->timestamp_pwm_us));
    nrfx_systick_get(&(context->timestamp_pwm_ms));
    context->pwm_state_on = state_on;
    context->pwm_action = action;
    context->delay_total = total_time;
    context->pwm_gpio_percent_ms = total_time / 100 / 2;
    context->pwm_duty_delay_us = 1000000 / frequency;
    context->current_ms = 0;
    context->pwm_percentage = 0;
    context->pwm_on_time = 0;
}
