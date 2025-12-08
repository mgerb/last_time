#pragma once

#include "common.h"
#include "pebble.h"

extern const int TIME_CONTAINER_HEIGHT;

void time_update(void);
void time_update_sunrise(time_t sunrise);
void time_update_sunset(time_t sunset);
void time_load(Window *window);
void time_unload(void);
