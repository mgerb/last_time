#include "settings.h"

#define SETTINGS_PERSIST_KEY 2

AppSettings app_settings;

static void settings_set_defaults(void) {
    snprintf(app_settings.temperature, sizeof(app_settings.temperature), "%s", "f");
    snprintf(app_settings.date_format, sizeof(app_settings.date_format), "%s", "MM-DD");
    app_settings.vibrate_disconnect = true;
    app_settings.vibrate_top_hour = false;
    app_settings.weather_update_interval = 30;
}

void settings_load(void) {
    if (!persist_exists(SETTINGS_PERSIST_KEY)) {
        settings_set_defaults();
        return;
    }

    AppSettings loaded;
    const int bytes = persist_read_data(SETTINGS_PERSIST_KEY, &loaded, sizeof(loaded));
    if (bytes != (int)sizeof(loaded)) {
        settings_set_defaults();
        return;
    }

    app_settings = loaded;
}

static void settings_save(void) {
    persist_write_data(SETTINGS_PERSIST_KEY, &app_settings, sizeof(AppSettings));
}

void settings_update_from_message(DictionaryIterator *iter) {
    const Tuple *temperature_tuple = dict_find(iter, MESSAGE_KEY_config_temperature);
    snprintf(app_settings.temperature, sizeof(app_settings.temperature), "%s", temperature_tuple->value->cstring);

    const Tuple *date_format_tuple = dict_find(iter, MESSAGE_KEY_config_date_format);
    snprintf(app_settings.date_format, sizeof(app_settings.date_format), "%s", date_format_tuple->value->cstring);

    const Tuple *vibrate_tuple = dict_find(iter, MESSAGE_KEY_config_vibrate_disconnect);
    app_settings.vibrate_disconnect = vibrate_tuple->value->int32 == 1;

    const Tuple *vibrate_top_hour_tuple = dict_find(iter, MESSAGE_KEY_config_vibrate_top_hour);
    app_settings.vibrate_top_hour = vibrate_top_hour_tuple->value->int32 == 1;

    const Tuple *weather_update_interval_tuple = dict_find(iter, MESSAGE_KEY_config_weather_update_interval);
    app_settings.weather_update_interval = weather_update_interval_tuple->value->int32;

    settings_save();
}
