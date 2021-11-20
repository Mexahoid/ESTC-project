#include "color.h"
#include <math.h>

// Current color mode.
static color_mode_t mode;
// Current HSV color
static color_hsv_t current;
// Increase/decrease values for model.
static color_hsv_t current_incdec;
// Timestamp for smooth changing.
static nrfx_systick_state_t timestamp_pwm_us;

void color_init()
{
    mode = CLR_OFF;
    current.h = COLOR_HUE_DEFAULT;
    current.s = COLOR_SAT_DEFAULT;
    if (current.s > 100)
        current.s = 100;
    if (current.s < 0)
        current.s = 0;
    current.v = COLOR_BRI_DEFAULT;
    if (current.v > 100)
        current.v = 100;
    if (current.v < 0)
        current.v = 0;

    current.h = current.h % 360;
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
    case CLR_OFF:
        break;
    case CLR_HUE:
        current.h += current_incdec.h;
        if (current.h >= 360)
            current.h = 0;
        break;
    case CLR_SAT:
        current.s += current_incdec.s;
        if (current.s >= 100 || current.s <  1)
            current_incdec.s *= -1;
        break;
    case CLR_BRI:
        current.v += current_incdec.v;
        if (current.v >= 100 || current.v < 1)
            current_incdec.v *= -1;
        break;
    }
}

void color_change_mode()
{
    if (mode == CLR_BRI)
        mode = CLR_OFF;
    else
        mode++;
}

color_mode_t color_get_mode()
{
    return mode;
}

void color_get_current_pwm_percentages(color_pwm_t *color)
{
    int hi = (int)(current.h / 60) % 6;
    double vmin = (100.0 - current.s) * current.v / 100.0;
    double a = (current.v - vmin) * (current.h % 60) / 60.0;
    double vinc = vmin + a;
    double vdec = current.v - a;

    double r = 0, g = 0, b = 0;

    switch (hi)
    {
    case 0:
        r = current.v;
        g = vinc;
        b = vmin;
        break;

    case 1:
        r = vdec;
        g = current.v;
        b = vmin;
        break;

    case 2:
        r = vmin;
        g = current.v;
        b = vinc;
        break;

    case 3:
        r = vmin;
        g = vdec;
        b = current.v;
        break;

    case 4:
        r = vinc;
        g = vmin;
        b = current.v;
        break;

    case 5:
        r = current.v;
        g = vmin;
        b = vdec;
        break;
    }

    color->r = (int)r;
    color->g = (int)g;
    color->b = (int)b;
}
