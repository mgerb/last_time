#pragma once

#include <pebble.h>

void battery_init();
void battery_load(Window *window, int row_height);
void battery_unload();
void battery_deinit();
