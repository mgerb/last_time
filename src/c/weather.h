#pragma once

#include "common.h"
#include "pebble.h"

void weather_request_if_needed(void);
void weather_update_condition_icon(void);
void weather_inbox_received_callback(DictionaryIterator *iterator, void *context);
void weather_request_reset_state(void);
void weather_load(Window *window);
void weather_unload(void);
