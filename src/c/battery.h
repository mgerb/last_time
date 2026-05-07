#pragma once

#include <pebble.h>

#define BATTERY_TEXT_RIGHT_INSET 2

extern int battery_row_max_width;

void battery_init();
void battery_load(Window *window, int row_height);
void battery_unload();
void battery_deinit();
