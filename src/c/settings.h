#pragma once

#include <pebble.h>

typedef struct {
    char temperature[16];
    char date_format[16];
    bool vibrate_disconnect;
    bool vibrate_top_hour;
    /** In minutes. */
    uint32_t weather_update_interval;
} AppSettings;

void settings_load(void);
extern AppSettings app_settings;
void settings_update_from_message(DictionaryIterator *iter);
