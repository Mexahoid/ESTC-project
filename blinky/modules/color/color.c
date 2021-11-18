#include "color.h"
#include <math.h>

// Current color mode.
color_mode_t color_mode = NOP;

static color_hsv_t color_current;

static color_hsv_t color_current_incdec;

void color_init()
{
    color_current_incdec.h = 1;
    color_current_incdec.s = 1;
    color_current_incdec.v = 1;
}

void color_mode_increase()
{
    switch (color_mode)
    {
    case NOP:
        break;
    case HUE:
        color_current.h += color_current_incdec.h;
        if (color_current.h >= 360)
            color_current.h = 0;
        break;
    case SAT:
        color_current.s += color_current_incdec.s;
        if (color_current.s >= 100)
            color_current_incdec.s = -1;
        if (color_current.s <= 0)
            color_current_incdec.s = 1;
        break;
    case BRI:
        color_current.v += color_current_incdec.v;
        if (color_current.v >= 100)
            color_current_incdec.v = -1;
        if (color_current.v <= 0)
            color_current_incdec.v = 1;
        break;
    }
}

void color_change_mode()
{
    if (color_mode == BRI)
        color_mode = NOP;
    else
        color_mode++;
}

void color_convert(color_pwm_t *color)
{
    int hi = (int)(color_current.h / 60) % 6;
    double vmin = (100.0 - color_current.s) * color_current.v / 100.0;
    double a = (color_current.v - vmin) * (color_current.h % 60) / 60.0;
    double vinc = vmin + a;
    double vdec = color_current.v - a;

    double r = 0, g = 0, b = 0;

    switch (hi)
    {
    case 0:
        r = color_current.v;
        g = vinc;
        b = vmin;
        break;

    case 1:
        r = vdec;
        g = color_current.v;
        b = vmin;
        break;

    case 2:
        r = vmin;
        g = color_current.v;
        b = vinc;
        break;

    case 3:
        r = vmin;
        g = vdec;
        b = color_current.v;
        break;

    case 4:
        r = vinc;
        g = vmin;
        b = color_current.v;
        break;

    case 5:
        r = color_current.v;
        g = vmin;
        b = vdec;
        break;
    }

    color->r = (int)r;
    color->g = (int)g;
    color->b = (int)b;
}
