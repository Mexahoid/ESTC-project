#include "color.h"
#include <math.h>

color_rgb_t color_convert(color_hsv_t input)
{
    color_rgb_t real_color;

    int c = input.s * input.v;
    int h_segm = input.h / 60;
    int x = c * (100 - fabs((h_segm) % 2 - 100));
    int m = input.v - c;

    double r = 0, g = 0, b = 0;

    switch (h_segm)
    {
        case 0:
            r = c;
            g = x;
            b = 0;
        break;

        case 1:
            r = x;
            g = c;
            b = 0;
        break;

        case 2:
            r = 0;
            g = c;
            b = x;
        break;

        case 3:
            r = 0;
            g = x;
            b = c;
        break;

        case 4:
            r = x;
            g = 0;
            b = c;
        break;

        case 5:
            r = c;
            g = 0;
            b = x;
        break;
    }

    real_color.r = (r + m) * 255 / 100;
    real_color.g = (g + m) * 255 / 100;
    real_color.b = (b + m) * 255 / 100;


    return real_color;
}