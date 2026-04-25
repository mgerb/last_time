#pragma once

#include <pebble.h>

#if defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_EMERY)
#define HEART_RATE_SUPPORTED
#endif

void health_init();
void health_load(Window *window, int row_height);
void health_unload();
void health_deinit();
