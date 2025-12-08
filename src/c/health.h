#pragma once

#include <pebble.h>

void health_init();
void health_load(Window *window, int row_height);
void health_unload();
void health_deinit();
