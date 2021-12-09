#ifndef MY_MODULE_COLOR
#define MY_MODULE_COLOR

#include "nrfx_systick.h"


// HSV color 360-100-100.
typedef struct {
    int32_t h;
    int32_t s;
    int32_t v;
} color_hsv_t;

// PWM color 100-100-100.
typedef struct {
    int32_t r;
    int32_t g;
    int32_t b;
} color_pwm_t;

// RGB color 255-255-255.
typedef struct {
    int32_t r;
    int32_t g;
    int32_t b;
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

/**
 * @brief Converts HSV model color to PWM model.
 *
 * @param color                     PWM color model
 */
void color_get_current_pwm_percentages(color_pwm_t* const color);

/**
 * @brief Changes mode NOP - HUE - SAT - BRI - NOP.
 *
 */
void color_change_mode();

/**
 * @brief Changes value based on current mode.
 *
 */
void color_increase_mode_value();

/**
 * @brief Returns current color mode.
 *
 * @return color_mode_t
 */
color_mode_t color_get_mode();

/**
 * @brief Returns current RGB model color.
 *
 * @param color                 RGB color model
 */
void color_get_current_rgb(color_rgb_t* const color);

/**
 * @brief Converts RGB to HSV.
 *
 * @param rgb                   Input RGB color model
 * @param hsv                   Output HSV color model
 */
void color_convert_rgb_hsv(color_rgb_t* const rgb, color_hsv_t* const hsv);

/**
 * @brief Initializes color module.
 *
 * @param state                 Initial HSV color model state
 */
void color_init(color_hsv_t* const state);

/**
 * @brief Sets current RGB color.
 *
 * @param rgb                   Input RGB color model
 */
void color_set_rgb(color_rgb_t* const rgb);

/**
 * @brief Sets current HSV color.
 *
 * @param hsv                   Input HSV color model
 */
void color_set_hsv(color_hsv_t* const hsv);

#endif