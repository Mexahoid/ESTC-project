#ifndef MY_MODULE_COLOR
#define MY_MODULE_COLOR

#include "nrfx_systick.h"

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

// RGB color 255-255-255.
typedef struct {
    int r;
    int g;
    int b;
} color_rgb_t;

// Color modes.
typedef enum {
    // No input (LED1 is always off)​.
    COLOR_MODE_OFF,
    COLOR_MODE_FIRST = COLOR_MODE_OFF,
    // Hue modification (LED1 blinks slowly)​.
    COLOR_MODE_HUE,
    // Saturation modification (LED1 blinks fast)​.
    COLOR_MODE_SAT,
    // Brightness modification (LED1 is always on)​.
    COLOR_MODE_BRI,
    COLOR_MODE_LAST = COLOR_MODE_BRI
} color_mode_t;

// Converts HSV model color to PWM model.
void color_get_current_pwm_percentages(color_pwm_t* const color);

// Changes mode NOP - HUE - SAT - BRI - NOP.
void color_change_mode();

// Changes value based on current mode.
void color_increase_mode_value();

// Returns current color mode.
color_mode_t color_get_mode();

// Returns current RGB model color.
void color_get_current_rgb(color_rgb_t* const color);

// Initializes color module.
void color_init(color_rgb_t* const state);

#endif