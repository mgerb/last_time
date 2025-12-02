#pragma once

#include "common.h"
#include "font.h"
#include "time.h"

#define WEATHER_CACHE_KEY 1
#define WEATHER_CACHE_TTL_SECONDS (30 * 60)
#define WEATHER_CACHE_CONDITION_LEN 20

static TextLayer *s_weather_layer_icon;
static TextLayer *s_condition_layer;
static TextLayer *s_temperature_layer;

static char temperature_buffer[8] = "--";
static char condition_buffer[20] = "--";

static const int WEATHER_GAP = 2;

typedef struct {
    int32_t temperature_f;
    char condition[WEATHER_CACHE_CONDITION_LEN];
    time_t timestamp;
} WeatherCache;

static bool s_weather_request_in_progress = false;

static bool weather_cache_load(WeatherCache *cache) {
    if (!persist_exists(WEATHER_CACHE_KEY)) {
        return false;
    }

    int bytes = persist_read_data(WEATHER_CACHE_KEY, cache, sizeof(*cache));
    return bytes == (int)sizeof(*cache);
}

static bool weather_cache_is_valid(const WeatherCache *cache) {
    time_t now = time(NULL);
    return difftime(now, cache->timestamp) < WEATHER_CACHE_TTL_SECONDS;
}

static void weather_cache_save(int32_t temperature_f, const char *condition) {
    WeatherCache cache = {.temperature_f = temperature_f, .timestamp = time(NULL)};
    snprintf(cache.condition, sizeof(cache.condition), "%s", condition);
    persist_write_data(WEATHER_CACHE_KEY, &cache, sizeof(cache));
}

/**
 * Check if weather is cached and valid, then apply to static buffers.
 */
static bool weather_load_and_apply_cache(void) {
    WeatherCache cache;
    if (!weather_cache_load(&cache) || !weather_cache_is_valid(&cache)) {
        return false;
    }

    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", (int)cache.temperature_f);
    snprintf(condition_buffer, sizeof(condition_buffer), "%s", cache.condition);
    return true;
}

/**
 * Use app_message to send a request to get the weather from the JS side of things.
 */
static void weather_send_request(void) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox begin failed: %d", (int)result);
        s_weather_request_in_progress = false;
        return;
    }

    dict_write_uint8(iter, 0, 0);

    result = app_message_outbox_send();
    if (result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed: %d", (int)result);
        s_weather_request_in_progress = false;
        return;
    }

    s_weather_request_in_progress = true;
}

static void weather_request_if_needed(void) {
    WeatherCache cache;
    bool has_cache = weather_cache_load(&cache);
    bool cache_valid = has_cache && weather_cache_is_valid(&cache);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "cache_valid: %d, s_weather_request_in_progress: %d", cache_valid,
            s_weather_request_in_progress);
    if (!cache_valid && !s_weather_request_in_progress) {
        weather_send_request();
    }
}

void position_weather_icon(void) {
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

static void weather_inbox_received_callback(DictionaryIterator *iterator, void *context) {
    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_temperature_f);
    Tuple *condition_tuple = dict_find(iterator, MESSAGE_KEY_condition);

    if (!temp_tuple || !condition_tuple) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "inbox_received_callback missing data (temp %p, condition %p)", temp_tuple,
                condition_tuple);
        return;
    }

    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", (int)temp_tuple->value->int32);
    snprintf(condition_buffer, sizeof(condition_buffer), "%s", condition_tuple->value->cstring);
    text_layer_set_text(s_temperature_layer, temperature_buffer);
    text_layer_set_text(s_condition_layer, condition_buffer);
    position_weather_icon();

    weather_cache_save(temp_tuple->value->int32, condition_tuple->value->cstring);
    s_weather_request_in_progress = false;
}

static void weather_load(Window *window) {
    weather_load_and_apply_cache();
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int temperature_row_height = 24;
    int conditions_row_height = 16;

    // Temperature text (top-left).
    s_temperature_layer = text_layer_create(GRect(PADDING_X, 0, 80, temperature_row_height));
    text_layer_set_font(s_temperature_layer, s_font_primary_small);
    text_layer_set_text_color(s_temperature_layer, THEME.text_color);
    text_layer_set_background_color(s_temperature_layer, GColorClear);
    text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentLeft);
    text_layer_set_text(s_temperature_layer, temperature_buffer);
    layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));

    // Weather icon sits to the right of the temperature.
    s_weather_layer_icon = font_render_icon(window_layer, ICON_CLOUDY, PADDING_X, 0, false, false);
    text_layer_set_text_color(s_weather_layer_icon, THEME.text_color);
    position_weather_icon();

    // Weather condition under the temperature/icon.
    s_condition_layer =
        text_layer_create(GRect(PADDING_X, (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2) - conditions_row_height - 2,
                                bounds.size.w, conditions_row_height));
    text_layer_set_font(s_condition_layer, s_font_primary_small);
    text_layer_set_text_color(s_condition_layer, THEME.text_color);
    text_layer_set_background_color(s_condition_layer, GColorClear);
    text_layer_set_text_alignment(s_condition_layer, GTextAlignmentLeft);
    text_layer_set_text(s_condition_layer, condition_buffer);
    layer_add_child(window_layer, text_layer_get_layer(s_condition_layer));
}

static void weather_unload() {
    text_layer_destroy(s_condition_layer);
    text_layer_destroy(s_temperature_layer);
    text_layer_destroy(s_weather_layer_icon);
}
