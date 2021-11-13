#ifndef MY_MODULE_PWM
#define MY_MODULE_PWM

#include "nrf_delay.h"
#include "nrfx_systick.h"

#define PWM_FREQUENCY 1000

// Modulates PWM in relation to timestamp. Checks whether the difference between timestamp and present tick is more than pwm_delay_passed_us and ONs or OFFs GPIO.
void pwm_modulate_related(int pwm_delay_passed_us, int gpio, nrfx_systick_state_t *timestamp);

// Modulates GPIO as PWM on pwm_duty_delay_us for a certain pwm_percentage.
void pwm_modulate(int gpio);

// Delay in us.
void pwm_delay_us(int amount);

void pwm_percentage_recalc(int current_ms, int half, int pwm_percent_delay_ms);

void pwm_init(void (*action)(int, int), int state_on);

#endif