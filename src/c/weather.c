#include "weather.h"
#include "font.h"
#include "log.h"
#include "moon.h"
#include "settings.h"
#include "time.h"

#include <limits.h>

static TextLayer *s_weather_layer_icon;
static TextLayer *s_condition_layer;
static TextLayer *s_temperature_layer;

static char s_condition_buffer[20] = "--";
static int32_t s_weather_code = -1;
static int32_t s_moon_phase = -1;
static int32_t s_temperature_f = INT32_MIN;
int32_t weather_sunrise = 0;
int32_t weather_sunset = 0;

static const int WEATHER_GAP = 2;

static bool weather_cache_load(WeatherCache *cache);

static void weather_cache_invalidate(void) {
    WeatherCache cache;
    if (!weather_cache_load(&cache)) {
        return;
    }

    cache.timestamp = 0;
    persist_write_data(WEATHER_CACHE_KEY, &cache, sizeof(cache));
}

static void weather_position_icon(void) {
    if (!s_weather_layer_icon || !s_temperature_layer) {
        return;
    }

    // Place the icon immediately to the right of the temperature text with a fixed gap.
    GSize temp_size = text_layer_get_content_size(s_temperature_layer);
    Layer *temp_layer = text_layer_get_layer(s_temperature_layer);
    GRect temp_frame = layer_get_frame(temp_layer);

    Layer *icon_layer = text_layer_get_layer(s_weather_layer_icon);
    GRect icon_frame = layer_get_frame(icon_layer);
    layer_set_frame(icon_layer, GRect(temp_frame.origin.x + temp_size.w + WEATHER_GAP, icon_frame.origin.y,
                                      icon_frame.size.w, icon_frame.size.h));
}

static int32_t weather_convert_f_to_c(int32_t temperature_f) {
    int32_t scaled = (temperature_f - 32) * 5;
    if (scaled >= 0) {
        return (scaled + 4) / 9;
    }

    return (scaled - 4) / 9;
}

void weather_refresh_temperature(void) {
    if (!s_temperature_layer) {
        return;
    }

    static char buffer[8] = "--";
    if (s_temperature_f != INT32_MIN) {
        int32_t display_temp =
            app_settings.temperature[0] == 'c' ? weather_convert_f_to_c(s_temperature_f) : s_temperature_f;
        snprintf(buffer, sizeof(buffer), "%d°", (int)display_temp);
    } else {
        snprintf(buffer, sizeof(buffer), "%s", "--");
    }

    text_layer_set_text(s_temperature_layer, buffer);
    weather_position_icon();
}

bool weather_cache_load(WeatherCache *cache) {
    if (!persist_exists(WEATHER_CACHE_KEY)) {
        return false;
    }

    int bytes = persist_read_data(WEATHER_CACHE_KEY, cache, sizeof(*cache));
    return bytes == (int)sizeof(*cache);
}

bool weather_cache_is_valid(const WeatherCache *cache) {
    time_t now = time(NULL);
    int32_t ttl_seconds = (int32_t)app_settings.weather_update_interval * 60;
    double age_seconds = difftime(now, cache->timestamp);
    if (age_seconds < 0) {
        LOG_WARN("Weather cache timestamp is in the future. Invalidating.");
        return false;
    }

    return age_seconds < ttl_seconds;
}

void weather_cache_save(int32_t temperature_f, const char *condition, int32_t weather_code, int32_t sunrise,
                        int32_t sunset, int32_t moon_phase) {
    WeatherCache cache = {
        .temperature_f = temperature_f,
        .timestamp = time(NULL),
        .weather_code = weather_code,
        .sunrise = sunrise,
        .sunset = sunset,
        .moon_phase = moon_phase,
    };
    snprintf(cache.condition, sizeof(cache.condition), "%s", condition);
    persist_write_data(WEATHER_CACHE_KEY, &cache, sizeof(cache));
}

/**
 * Check if weather is cached and valid, then apply to static buffers.
 */
bool weather_load_and_apply_cache(void) {
    WeatherCache cache;
    if (!weather_cache_load(&cache) || !weather_cache_is_valid(&cache)) {
        LOG_DEBUG("weather cache is not valid");
        return false;
    }

    s_temperature_f = cache.temperature_f;
    snprintf(s_condition_buffer, sizeof(s_condition_buffer), "%s", cache.condition);
    s_weather_code = cache.weather_code;
    weather_sunrise = cache.sunrise;
    weather_sunset = cache.sunset;
    s_moon_phase = cache.moon_phase;
    return true;
}

/**
 * Use app_message to send a request to get the weather from the JS side of things.
 */
void weather_send_request(void) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result != APP_MSG_OK) {
        LOG_ERROR("Outbox begin failed: %d", (int)result);
        return;
    }

    dict_write_uint8(iter, 0, 0);

    result = app_message_outbox_send();
    if (result != APP_MSG_OK) {
        LOG_ERROR("Outbox send failed: %d", (int)result);
        return;
    }
}

static void weather_request_if_needed(void) {
    WeatherCache cache;
    bool has_cache = weather_cache_load(&cache);
    bool cache_valid = has_cache && weather_cache_is_valid(&cache);

    if (!cache_valid) {
        LOG_DEBUG("Weather cache is invalid; sending weather request.");
        weather_send_request();
    } else {
        LOG_DEBUG("Weather cache is valid; skipping weather request.");
    }
}

bool weather_is_night(void) {
    time_t now = time(NULL);
    LOG_DEBUG("now: %d, sunrise: %d, sunset: %d", (int)now, (int)weather_sunrise, (int)weather_sunset);
    // We only show sunrise/sunset times in the future, so it
    // should always be night when the next sunrise is before the next sunset.
    return weather_sunrise < weather_sunset;
}

// NOTE: See weatherCodeToText in index.js for descriptions.
char *weather_get_condition_icon(void) {
    switch (s_weather_code) {
    case 0:
    case 1: // Clear.
        return weather_is_night() ? "" : "";
    case 2:
    case 3: // Overcast.
        return weather_is_night() ? "" : "";
    case 45:
    case 48: // Fog.
        return weather_is_night() ? "" : "";
    case 51:
    case 53:
    case 55:
    case 61:
    case 63:
    case 65:
    case 80:
    case 81:
    case 82: // Rain.
        return weather_is_night() ? "" : "";
    case 56:
    case 57:
    case 66:
    case 67: // Rain/snow mix.
        return weather_is_night() ? "" : "";
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
        return weather_is_night() ? "" : "";
    case 95: // Thunderstorm.
        return weather_is_night() ? "" : "";
    case 96:
    case 97:
    case 98:
    case 99: // Hail storm.
        return weather_is_night() ? "" : "";
    default: // Clear (should never occur).
        return weather_is_night() ? "" : "";
    }
}

/**
 * Update the weather icon in the tick handler, because
 * it changes based on day/night.
 */
static void weather_update_condition_icon(void) {
    text_layer_set_text(s_weather_layer_icon, weather_get_condition_icon());
}

void weather_inbox_received_callback(DictionaryIterator *iterator, void *context) {
    Tuple *error_tuple = dict_find(iterator, MESSAGE_KEY_error);
    if (error_tuple) {
        LOG_ERROR("Weather request failed on phone: %ld", (long)error_tuple->value->int32);
        return;
    }

    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_temperature_f);
    Tuple *condition_tuple = dict_find(iterator, MESSAGE_KEY_condition);
    Tuple *weather_code_tuple = dict_find(iterator, MESSAGE_KEY_weather_code);
    Tuple *sunrise_tuple = dict_find(iterator, MESSAGE_KEY_sunrise);
    Tuple *sunset_tuple = dict_find(iterator, MESSAGE_KEY_sunset);
    Tuple *moon_phase_tuple = dict_find(iterator, MESSAGE_KEY_moon_phase);

    s_temperature_f = temp_tuple->value->int32;
    snprintf(s_condition_buffer, sizeof(s_condition_buffer), "%s", condition_tuple->value->cstring);
    s_weather_code = weather_code_tuple->value->int32;
    weather_sunrise = sunrise_tuple->value->int32;
    weather_sunset = sunset_tuple->value->int32;
    s_moon_phase = moon_phase_tuple->value->int32;
    time_update_sunrise((time_t)weather_sunrise);
    time_update_sunset((time_t)weather_sunset);
    moon_update(s_moon_phase);

    weather_refresh_temperature();
    text_layer_set_text(s_condition_layer, s_condition_buffer);
    weather_update_condition_icon();

    weather_cache_save(temp_tuple->value->int32, condition_tuple->value->cstring, s_weather_code, weather_sunrise,
                       weather_sunset, s_moon_phase);
}

void weather_load(Window *window) {
    weather_load_and_apply_cache();
    time_update_sunrise((time_t)weather_sunrise);
    time_update_sunset((time_t)weather_sunset);
    moon_update(s_moon_phase);
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int temperature_row_height = 24;
    int conditions_row_height = 16;

    // Temperature text (top-left).
    s_temperature_layer = text_layer_create(GRect(PADDING_X, 2, 80, temperature_row_height));
    text_layer_set_font(s_temperature_layer, s_font_temperature);
    text_layer_set_text_color(s_temperature_layer, THEME.text_color);
    text_layer_set_background_color(s_temperature_layer, GColorClear);
    text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentLeft);
    weather_refresh_temperature();
    layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));

    // Weather icon sits to the right of the temperature.
    s_weather_layer_icon =
        font_render_icon_large(window_layer, weather_get_condition_icon(), PADDING_X, -3, false, false);
    text_layer_set_text_color(s_weather_layer_icon, THEME.text_color);
    weather_position_icon();

    // Weather condition under the temperature/icon.
    s_condition_layer = text_layer_create(
        GRect(PADDING_X, (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2) - conditions_row_height - 4, bounds.size.w,
              conditions_row_height));
    text_layer_set_font(s_condition_layer, s_font_primary_small);
    text_layer_set_text_color(s_condition_layer, THEME.text_color);
    text_layer_set_background_color(s_condition_layer, GColorClear);
    text_layer_set_text_alignment(s_condition_layer, GTextAlignmentLeft);
    text_layer_set_text(s_condition_layer, s_condition_buffer);
    layer_add_child(window_layer, text_layer_get_layer(s_condition_layer));
}

void weather_unload(void) {
    text_layer_destroy(s_condition_layer);
    text_layer_destroy(s_temperature_layer);
    text_layer_destroy(s_weather_layer_icon);
}

void weather_tick_handler(void) {
    weather_request_if_needed();
    weather_update_condition_icon();
}
