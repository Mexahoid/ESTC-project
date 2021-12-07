#include "color.h"
#include "helpers.h"
#include <math.h>

// Delay between incdec calls (in us).
#define COLOR_DELAY_US 45000

// Default value for hue.
#define COLOR_DEFAULT_HUE 0
// Default value for saturation.
#define COLOR_DEFAULT_SAT 100
// Default value for brightness.
#define COLOR_DEFAULT_BRI 100

// Current color mode.
static color_mode_t mode;
// Current HSV color
static color_hsv_t current;
// Increase/decrease values for model.
static color_hsv_t current_incdec;
// Timestamp for smooth changing.
static nrfx_systick_state_t timestamp_pwm_us;

// Converts RGB model to HSV model.
void color_convert_rgb_hsv(color_rgb_t* const rgb, color_hsv_t* const hsv)
{
    int r = rgb->r * 100 / 255;
    int g = rgb->g * 100 / 255;
    int b = rgb->b * 100 / 255;

    int max_p = helper_max(r, helper_max(g, b));
    int min_p = helper_min(r, helper_min(g, b));

    hsv->v = max_p;

    if (max_p == 0)
        hsv->s = 0;
    else
        hsv->s = 100 - (min_p * 100 / max_p); //Because it's not doubles.

    // Unknown
    if (max_p == min_p)
    {
        hsv->h = 0;
        return;
    }

    if (max_p == r && g >= b)
    {
        hsv->h = 60 * (g - b) / (max_p - min_p);
        return;
    }

    if (max_p == r && g < b)
    {
        hsv->h = 60 * (g - b) / (max_p - min_p) + 360;
        return;
    }

    if (max_p == g)
    {
        hsv->h = 60 * (b - r) / (max_p - min_p) + 120;
        return;
    }

    if (max_p == b)
    {
        hsv->h = 60 * (r - g) / (max_p - min_p) + 240;
        return;
    }
}

// Converts HSV model to PWM model.
static void convert_hsv_pwm(color_hsv_t* const hsv, color_pwm_t* const pwm)
{
    int hi = (int)(hsv->h / 60) % 6;
    double vmin = (100.0 - hsv->s) * hsv->v / 100.0;
    double a = (hsv->v - vmin) * (hsv->h % 60) / 60.0;
    double vinc = vmin + a;
    double vdec = hsv->v - a;

    double r = 0, g = 0, b = 0;

    switch (hi)
    {
    case 0:
        r = hsv->v;
        g = vinc;
        b = vmin;
        break;

    case 1:
        r = vdec;
        g = hsv->v;
        b = vmin;
        break;

    case 2:
        r = vmin;
        g = hsv->v;
        b = vinc;
        break;

    case 3:
        r = vmin;
        g = vdec;
        b = hsv->v;
        break;

    case 4:
        r = vinc;
        g = vmin;
        b = hsv->v;
        break;

    case 5:
        r = hsv->v;
        g = vmin;
        b = vdec;
        break;
    }

    pwm->r = (int)r;
    pwm->g = (int)g;
    pwm->b = (int)b;
}

// Converts PWM model to RGB model.
static void convert_pwm_rgb(color_pwm_t* const pwm, color_rgb_t* const rgb)
{
    rgb->r = pwm->r * 255 / 100;
    rgb->g = pwm->g * 255 / 100;
    rgb->b = pwm->b * 255 / 100;
}

// Converts HSV model to RGB model.
static void convert_hsv_rgb(color_hsv_t* const hsv, color_rgb_t* const rgb)
{
    color_pwm_t pwm;
    convert_hsv_pwm(hsv, &pwm);
    convert_pwm_rgb(&pwm, rgb);
}

void color_init(color_hsv_t* const state)
{
    mode = COLOR_MODE_OFF;
    int32_t hue = 0;
    int32_t sat = 0;
    int32_t bri = 0;
    if (state == NULL)
    {
        hue = helper_clamp(COLOR_DEFAULT_HUE, 0, 360);
        sat = helper_clamp(COLOR_DEFAULT_SAT, 0, 100);
        bri = helper_clamp(COLOR_DEFAULT_BRI, 0, 100);
    }
    else
    {
        //hue = 0;
        //sat = 0;
        //bri = 100;

        hue = helper_clamp(state->h, 0, 360);
        sat = helper_clamp(state->s, 0, 100);
        bri = helper_clamp(state->v, 0, 100);
    }

    current.h = hue;
    current.s = bri;
    current.v = sat;
    current_incdec.h = 1;
    current_incdec.s = current.s >= 100 ? -1 : 1;
    current_incdec.v = current.v >= 100 ? -1 : 1;
}

void color_increase_mode_value()
{
    if (!nrfx_systick_test(&timestamp_pwm_us, COLOR_DELAY_US))
        return;
    nrfx_systick_get(&timestamp_pwm_us);

    switch (mode)
    {
    case COLOR_MODE_OFF:
        break;
    case COLOR_MODE_HUE:
        current.h += current_incdec.h;
        if (current.h >= 360)
            current.h = 0;
        break;
    case COLOR_MODE_SAT:
        helper_increment_circular(&(current.s), &(current_incdec.s), 0, 100);
        break;
    case COLOR_MODE_BRI:
        helper_increment_circular(&(current.v), &(current_incdec.v), 0, 100);
        break;
    }
}

void color_change_mode()
{
    if (mode == COLOR_MODE_LAST)
        mode = COLOR_MODE_FIRST;
    else
        mode++;
}

color_mode_t color_get_mode()
{
    return mode;
}

void color_get_current_pwm_percentages(color_pwm_t* const color)
{
    convert_hsv_pwm(&current, color);
}

void color_get_current_rgb(color_rgb_t* const color)
{
    convert_hsv_rgb(&current, color);
}

void color_set_rgb(color_rgb_t* const rgb)
{
    color_convert_rgb_hsv(rgb, &current);
}

void color_set_hsv(color_hsv_t* const hsv)
{
    current.h = helper_clamp(hsv->h, 0, 360);
    current.s = helper_clamp(hsv->s, 0, 100);
    current.v = helper_clamp(hsv->v, 0, 100);
}