#include "settings.h"
#include <stdint.h>

// The settings version is used to manage migrations. When the
// settings struct changes, increment this version and then
// handle the migrations in settings_load.
#define SETTINGS_VERSION 2

#define SETTINGS_VERSION_KEY 10
#define SETTINGS_PERSIST_KEY 2

AppSettings app_settings;

static void settings_update_color_from_message(DictionaryIterator *iter, uint32_t key, GColor *color) {
    const Tuple *tuple = dict_find(iter, key);
    if (!tuple) {
        return;
    }

    *color = GColorFromHEX(tuple->value->int32);
}

static void settings_set_defaults(void) {
    snprintf(app_settings.temperature, sizeof(app_settings.temperature), "%s", "f");
    snprintf(app_settings.date_format, sizeof(app_settings.date_format), "%s", "MM-DD");
    snprintf(app_settings.date_separator, sizeof(app_settings.date_separator), "%s", "-");
    app_settings.vibrate_disconnect = true;
    app_settings.vibrate_top_hour = false;
    app_settings.show_steps = true;
    app_settings.weather_update_interval = 30;
    app_settings.show_heart_rate = false;
    app_settings.text_color = (GColor){.argb = GColorWhiteARGB8};
    app_settings.text_color_secondary = (GColor){.argb = GColorBlackARGB8};
    app_settings.bg_color = (GColor){.argb = GColorBlackARGB8};
    app_settings.bg_color_secondary = (GColor){.argb = GColorWhiteARGB8};
}

static void settings_save(void) {
    persist_write_data(SETTINGS_PERSIST_KEY, &app_settings, sizeof(AppSettings));
    // Always sync the version when writing settings.
    uint32_t version = SETTINGS_VERSION;
    persist_write_data(SETTINGS_VERSION_KEY, &version, sizeof(uint32_t));
}

void settings_load(void) {
    settings_set_defaults();

    if (!persist_exists(SETTINGS_PERSIST_KEY)) {
        return;
    }

    // Handle migrations here. If there is a version mismatch, then the default values
    // need to be set manually. The first migration doesn't require a version check,
    // because the version doesn't exist in storage yet.
    if (!persist_exists(SETTINGS_VERSION_KEY) && persist_get_size(SETTINGS_PERSIST_KEY) == sizeof(LegacyAppSettings1)) {
        LegacyAppSettings1 legacy;
        persist_read_data(SETTINGS_PERSIST_KEY, &legacy, sizeof(LegacyAppSettings1));

        snprintf(app_settings.temperature, sizeof(app_settings.temperature), "%s", legacy.temperature);
        snprintf(app_settings.date_format, sizeof(app_settings.date_format), "%s", legacy.date_format);
        snprintf(app_settings.date_separator, sizeof(app_settings.date_separator), "%s", legacy.date_separator);

        app_settings.vibrate_disconnect = legacy.vibrate_disconnect;
        app_settings.vibrate_top_hour = legacy.vibrate_top_hour;
        app_settings.show_steps = legacy.show_steps;
        app_settings.weather_update_interval = legacy.weather_update_interval;
        app_settings.show_heart_rate = legacy.show_heart_rate;

        // Saving the settings also syncs the version.
        settings_save();
    } else {
        uint32_t settings_version = 0;
        persist_read_data(SETTINGS_VERSION_KEY, &settings_version, sizeof(uint32_t));

        // This is the happy path.
        if (settings_version == SETTINGS_VERSION) {
            persist_read_data(SETTINGS_PERSIST_KEY, &app_settings, sizeof(app_settings));
        }
        // NOTE: Future migrations will be handled here by checking the settings_version.
    }
}

void settings_update_from_message(DictionaryIterator *iter) {
    const Tuple *temperature_tuple = dict_find(iter, MESSAGE_KEY_config_temperature);
    snprintf(app_settings.temperature, sizeof(app_settings.temperature), "%s", temperature_tuple->value->cstring);

    const Tuple *date_format_tuple = dict_find(iter, MESSAGE_KEY_config_date_format);
    snprintf(app_settings.date_format, sizeof(app_settings.date_format), "%s", date_format_tuple->value->cstring);

    const Tuple *date_separator_tuple = dict_find(iter, MESSAGE_KEY_config_date_separator);
    snprintf(app_settings.date_separator, sizeof(app_settings.date_separator), "%s",
             date_separator_tuple->value->cstring);

    const Tuple *vibrate_tuple = dict_find(iter, MESSAGE_KEY_config_vibrate_disconnect);
    app_settings.vibrate_disconnect = vibrate_tuple->value->int32 == 1;

    const Tuple *vibrate_top_hour_tuple = dict_find(iter, MESSAGE_KEY_config_vibrate_top_hour);
    app_settings.vibrate_top_hour = vibrate_top_hour_tuple->value->int32 == 1;

    const Tuple *show_steps_tuple = dict_find(iter, MESSAGE_KEY_config_show_steps);
    app_settings.show_steps = show_steps_tuple->value->int32 == 1;

    const Tuple *weather_update_interval_tuple = dict_find(iter, MESSAGE_KEY_config_weather_update_interval);
    app_settings.weather_update_interval = weather_update_interval_tuple->value->int32;

    const Tuple *show_heart_rate_tuple = dict_find(iter, MESSAGE_KEY_config_show_heart_rate);
    app_settings.show_heart_rate = show_heart_rate_tuple->value->int32 == 1;

    settings_update_color_from_message(iter, MESSAGE_KEY_config_text_color, &app_settings.text_color);
    settings_update_color_from_message(iter, MESSAGE_KEY_config_text_color_secondary,
                                       &app_settings.text_color_secondary);
    settings_update_color_from_message(iter, MESSAGE_KEY_config_bg_color, &app_settings.bg_color);
    settings_update_color_from_message(iter, MESSAGE_KEY_config_bg_color_secondary, &app_settings.bg_color_secondary);

    settings_save();
}
