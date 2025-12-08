#pragma once

#include <pebble.h>

extern Window *s_window;

typedef struct {
    GColor text_color;
    GColor text_color_secondary;
    GColor bg_color;
    GColor bg_color_secondary;
} Theme;

extern const int PADDING_X;
extern const Theme THEME;
