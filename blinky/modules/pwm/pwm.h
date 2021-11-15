#ifndef MY_MODULE_PWM
#define MY_MODULE_PWM

#include "nrfx_systick.h"

// Frequency of PWM in kHz.
#define PWM_FREQUENCY 1000
// Delay for the inner counter.
#define PWM_US_DELAY 1
// Max inner counter counts before nullying.
#define PWM_COUNTER_MAX 1000
// Max amount of ms in s (1000 lol).
#define PWM_MAX_MS_DELAY 1000

// Modulates GPIO with PWM.
void pwm_modulate(int gpio);

// Looks every PWM ms for a PWM percentage recalc.
void pwm_percentage_recalc();

// Initializes PWM module.
void pwm_init(void (*action)(int, int), int state_on, int on_time);

// Updates inner counter every us.
void pwm_tick_update();

// Returns true if PWM total delay has passed.
bool pwm_is_delay_passed();

#endif