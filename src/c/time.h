#pragma once

#include "common.h"
#include "pebble.h"

extern const int TIME_CONTAINER_HEIGHT;

void time_update(void);
void time_load(Window *window);
void time_unload(void);
