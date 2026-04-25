#pragma once

#include <pebble.h>

#if defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_EMERY)
#define HEART_RATE_SUPPORTED
#endif

void health_load(Window *window, int row_height);
void health_unload(void);
void health_sync_service(void);
void health_deinit(void);
