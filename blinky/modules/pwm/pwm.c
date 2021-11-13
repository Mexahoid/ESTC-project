#include "pwm.h"

int pwm_percentage = 0;

int pwm_delay_ms = 0;
int pwm_duty_delay_us = 1000000 / PWM_FREQUENCY;

void (*pwm_action)(int, int);
int pwm_state_on;

void pwm_delay_us(int amount)
{
    nrf_delay_us(amount);
}

// Modulates PWM in relation to timestamp. Checks whether the difference between timestamp and present tick is more than pwm_delay_passed_us and ONs or OFFs GPIO.
void pwm_modulate_related(int pwm_delay_passed_us, int gpio, nrfx_systick_state_t *timestamp)
{
    if (!nrfx_systick_test(timestamp, pwm_delay_passed_us))
        pwm_action(gpio, pwm_state_on);
    else
        pwm_action(gpio, !pwm_state_on);
}

// Modulates GPIO as PWM on pwm_duty_delay_us for a certain pwm_percentage.
void pwm_modulate(int gpio)
{
    nrfx_systick_state_t timestamp_pwm;
    nrfx_systick_get(&timestamp_pwm);
    if (pwm_percentage <= 100 && pwm_percentage >= 0)
    {
        int frac = pwm_duty_delay_us / 100;
        nrfx_systick_get(&timestamp_pwm);
        for (int i = 0; i < pwm_duty_delay_us; i += frac)
        {
            pwm_modulate_related(pwm_percentage * frac, gpio, &timestamp_pwm);
            pwm_delay_us(frac);
        }
    }
}

void pwm_percentage_recalc(int current_ms, int half, int pwm_percent_delay_ms)
{
    if (pwm_delay_ms >= pwm_percent_delay_ms)
    {
        // for first 500 ms pwm increases, for second one - decreases
        if (current_ms <= half && pwm_percentage < 100)
            pwm_percentage++;
        else
            pwm_percentage--;
        pwm_delay_ms = 0;
    }

    pwm_delay_ms++;
}

void pwm_init(void (*action)(int, int), int state_on)
{
    nrfx_systick_init();
    pwm_state_on = state_on;
    pwm_action = action;
}