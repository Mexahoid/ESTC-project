#ifndef MY_MODULE_PWM
#define MY_MODULE_PWM

#include "nrfx_systick.h"

#define PWM_FREQUENCY 1000

// Modulates PWM in relation to timestamp. Checks whether the difference between timestamp and present tick is more than pwm_delay_passed_us and ONs or OFFs GPIO.
void pwm_modulate_related(int pwm_delay_passed_us, int gpio, nrfx_systick_state_t *timestamp);

// Modulates GPIO as PWM on pwm_duty_delay_us for a certain pwm_percentage.
void pwm_modulate(int gpio);

void pwm_percentage_recalc();

void pwm_init(void (*action)(int, int), int state_on, int on_time);

void pwm_tick_update();

bool pwm_is_ms_passed();

#endif