#pragma once

#include <pebble.h>

typedef struct {
    char temperature[16];
    char date_format[16];
    char date_separator[4];
    bool vibrate_disconnect;
    bool vibrate_top_hour;
    bool show_steps;
    /** In minutes. */
    uint32_t weather_update_interval;
    bool show_heart_rate;
} LegacyAppSettings1;

// NOTE: AppSettings is always current. Legacy settings will get
// created to handle migrations.
typedef struct {
    char temperature[16];
    char date_format[16];
    char date_separator[4];
    bool vibrate_disconnect;
    bool vibrate_top_hour;
    bool show_steps;
    /** In minutes. */
    uint32_t weather_update_interval;
    bool show_heart_rate;
    GColor text_color;
    GColor text_color_secondary;
    GColor bg_color;
    GColor bg_color_secondary;
} AppSettings;

void settings_load(void);
extern AppSettings app_settings;
void settings_update_from_message(DictionaryIterator *iter);
