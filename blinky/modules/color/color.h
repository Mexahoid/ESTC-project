#ifndef MY_MODULE_COLOR
#define MY_MODULE_COLOR

// RGB color 255-255-255
typedef struct {
    int r;
    int g;
    int b;
} color_rgb_t;

// HSV color 360-100-100
typedef struct {
    int h;
    int s;
    int v;
} color_hsv_t;

// PWM color 100-100-100
typedef struct {
    int r;
    int g;
    int b;
} color_pwm_t;

// Color modes.
typedef enum {
    // No input (LED1 is always off)​
    NOP,
    // Hue modification (LED1 blinks slowly)​
    HUE,
    // Saturation modification (LED1 blinks fast)​
    SAT,
    // Brightness modification (LED1 is always on)​
    BRI
} color_mode_t;

color_rgb_t color_convert(color_hsv_t input);

color_pwm_t color_convert_pwm(color_rgb_t input);

// Changes mode NOP - HUE - SAT - BRI - NOP
void color_change_mode();

#endif