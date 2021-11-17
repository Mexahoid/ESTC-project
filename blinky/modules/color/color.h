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

color_rgb_t color_convert(color_hsv_t input);

#endif