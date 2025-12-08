#include "common.h"

Window *s_window;

const int PADDING_X = 4;
const Theme THEME = {
    .text_color = {.argb = GColorWhiteARGB8},           // GColorWhite
    .text_color_secondary = {.argb = GColorBlackARGB8}, // GColorBlack
    .bg_color = {.argb = GColorBlackARGB8},             // GColorWhite
    .bg_color_secondary = {.argb = GColorWhiteARGB8},   // GColorBlack
};
