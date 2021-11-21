#ifndef MY_MODULE_COLOR
#define MY_MODULE_COLOR

#include "nrfx_systick.h"

// Delay between incdec calls (in us).
#define COLOR_DELAY_US 45000

// Default values for init.
#define COLOR_HUE_DEFAULT 0
#define COLOR_SAT_DEFAULT 100
#define COLOR_BRI_DEFAULT 100

// HSV color 360-100-100.
typedef struct {
    int h;
    int s;
    int v;
} color_hsv_t;

// PWM color 100-100-100.
typedef struct {
    int r;
    int g;
    int b;
} color_pwm_t;

// Color modes.
typedef enum {
    // No input (LED1 is always off)​.
    CLR_OFF,
    CLR_FIRST = CLR_OFF,
    // Hue modification (LED1 blinks slowly)​.
    CLR_HUE,
    // Saturation modification (LED1 blinks fast)​.
    CLR_SAT,
    // Brightness modification (LED1 is always on)​.
    CLR_BRI,
    CLR_LAST = CLR_BRI
} color_mode_t;

// Converts HSV model color to PWM model.
void color_get_current_pwm_percentages(color_pwm_t *color);

// Changes mode NOP - HUE - SAT - BRI - NOP.
void color_change_mode();

// Changes value based on current mode.
void color_increase_mode_value();

// Returns current color mode.
color_mode_t color_get_mode();

// Initializes color module.
void color_init();

#endif