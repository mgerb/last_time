#pragma once

#include "common.h"
#include "pebble.h"

#define WEATHER_CACHE_KEY 1
#define WEATHER_CACHE_CONDITION_LEN 20
#define WEATHER_REQUEST_TIMEOUT_SECONDS 30

typedef struct {
    int32_t temperature_f;
    char condition[WEATHER_CACHE_CONDITION_LEN];
    time_t timestamp;
    int32_t weather_code;
    int32_t sunrise;
    int32_t sunset;
    int32_t moon_phase;
} WeatherCache;

void weather_inbox_received_callback(DictionaryIterator *iterator, void *context);
void weather_refresh_temperature(void);
void weather_load(Window *window);
void weather_unload(void);
void weather_tick_handler(void);
