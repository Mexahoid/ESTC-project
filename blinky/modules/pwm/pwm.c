#include "pwm.h"

// Was systic initialized or not.
static bool is_systick_init = false;

// Returns true if at least ms has passed.
static bool is_ms_passed(pwm_ctx_t* const context)
{
    if (!nrfx_systick_test(&(context->pwm_timestamp_ms), PWM_MAX_MS_DELAY))
        return false;
    nrfx_systick_get(&(context->pwm_timestamp_ms));
    context->current_ms++;
    return true;
}

// Updates inner counter every PWM percent delay.
static void tick_update(pwm_ctx_t* const context)
{
    if (!nrfx_systick_test(&(context->pwm_timestamp_us), PWM_TICK_DELAY_US))
        return;
    nrfx_systick_get(&(context->pwm_timestamp_us));

    context->pwm_counter += PWM_TICK_DELAY_US;
    if (context->pwm_counter >= PWM_COUNTER_MAX)
        context->pwm_counter = 0;

    if (context->current_ms >= context->delay_total)
    {
        context->current_ms = 0;
        context->pwm_percentage = 0;
    }
}


void pwm_modulate(pwm_ctx_t* const context)
{
    tick_update(context);
    if (context->pwm_counter < context->pwm_on_time)
        context->pwm_action(context->pwm_gpio, context->pwm_state_on);
    else
        context->pwm_action(context->pwm_gpio, !context->pwm_state_on);
}

void pwm_percentage_recalc(pwm_ctx_t* const context)
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

void pwm_set_percentage(pwm_ctx_t* const context, int percentage)
{
    context->pwm_percentage = percentage;
    context->pwm_on_time = context->pwm_duty_delay_us * context->pwm_percentage / 100;
}

void pwm_init(pwm_ctx_t* const context, void (*action)(int, int), int state_on, int total_time, int frequency, int gpio)
{
    memset(context, sizeof(*context), 0);

    if (!is_systick_init)
    {
        nrfx_systick_init();
        is_systick_init = true;
    }

    nrfx_systick_get(&(context->pwm_timestamp_us));
    nrfx_systick_get(&(context->pwm_timestamp_ms));
    context->pwm_gpio = gpio;
    context->pwm_state_on = state_on;
    context->pwm_action = action;
    context->delay_total = total_time;
    context->pwm_gpio_percent_ms = total_time / 100 / 2;
    context->pwm_duty_delay_us = 1000000 / frequency;
    context->current_ms = 0;
    context->pwm_percentage = 0;
    context->pwm_on_time = 0;
    context->pwm_counter = 0;
}
