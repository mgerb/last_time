#pragma once

#include <pebble.h>

static Window *s_window;
static TextLayer *s_battery_layer_text;
static TextLayer *s_battery_layer_icon;
#if defined(PBL_HEALTH)
static TextLayer *s_steps_layer_text;
static TextLayer *s_steps_layer_icon;
#endif

static const int PADDING_X = 4;

typedef struct {
    GColor text_color;
    GColor text_color_secondary;
    GColor bg_color;
    GColor bg_color_secondary;
} Theme;

static Theme THEME = {
    .text_color = {.argb = GColorWhiteARGB8},           // GColorWhite
    .text_color_secondary = {.argb = GColorBlackARGB8}, // GColorBlack
    .bg_color = {.argb = GColorBlackARGB8},             // GColorWhite
    .bg_color_secondary = {.argb = GColorWhiteARGB8},   // GColorBlack
};
