#pragma once

#include <pebble.h>

#define BATTERY_TEXT_RIGHT_INSET 2

extern int BATTERY_ROW_MAX_WIDTH;

void battery_init();
void battery_load(Window *window, int row_height);
void battery_unload();
void battery_deinit();
