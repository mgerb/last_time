#include "time.h"
#include "common.h"
#include "font.h"
#include "gcolor_definitions.h"
#include "pebble.h"
#include "settings.h"
#include <assert.h>

static TextLayer *s_time_layer;
static TextLayer *s_ampm_layer;
static Layer *s_time_layer_container;
static Layer *s_date_layer_container;
static TextLayer *s_sunrise_icon_layer;
static TextLayer *s_sunrise_layer;
static TextLayer *s_sunset_icon_layer;
static TextLayer *s_sunset_layer;
static TextLayer *s_utc_icon_layer;
static TextLayer *s_utc_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;
static char s_sunrise_buffer[6] = "--:--";
static char s_sunset_buffer[6] = "--:--";
static char s_ampm_buffer[3] = "";

const int TIME_CONTAINER_HEIGHT = 50;
const int UTC_ROW_HEIGHT = 22;
const int SOLAR_TIME_ROW_HEIGHT = 18;

static void format_time(char *buffer, size_t buffer_size, struct tm *time_value) {
    if (clock_is_24h_style()) {
        strftime(buffer, buffer_size, "%H:%M", time_value);
        return;
    }

    assert(buffer_size >= 2);

    // Trim off leading 0 in 12 hour mode.
    strftime(buffer, buffer_size, "%I:%M", time_value);
    if (buffer[0] == '0') {
        memmove(buffer, buffer + 1, buffer_size - 1);
    }
}

static void time_update_ampm(struct tm *time_value) {
    if (!s_ampm_layer) {
        return;
    }

    const bool is_24h = clock_is_24h_style();
    layer_set_hidden(text_layer_get_layer(s_ampm_layer), is_24h);

    if (is_24h) {
        return;
    }

    snprintf(s_ampm_buffer, sizeof(s_ampm_buffer), "%s", (time_value->tm_hour >= 12) ? "PM" : "AM");

    text_layer_set_text(s_ampm_layer, s_ampm_buffer);
}

static const char *time_get_date_format(void) {
    if (strcmp(app_settings.date_format, "YYYY-MM-DD") == 0) {
        return "%Y-%m-%d";
    }
    if (strcmp(app_settings.date_format, "MM/DD/YYYY") == 0) {
        return "%m/%d/%Y";
    }
    if (strcmp(app_settings.date_format, "DD/MM/YYYY") == 0) {
        return "%d/%m/%Y";
    }
    if (strcmp(app_settings.date_format, "YYYY/MM/DD") == 0) {
        return "%Y/%m/%d";
    }
    if (strcmp(app_settings.date_format, "DD.MM.YYYY") == 0) {
        return "%d.%m.%Y";
    }

    return "%m-%d";
}

static void time_update_date_and_day(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    static char date_buffer[20];
    static char day_buffer[10];

    const char *date_format = time_get_date_format();
    strftime(date_buffer, sizeof(date_buffer), date_format, t);
    strftime(day_buffer, sizeof(day_buffer), "%a", t);

    text_layer_set_text(s_date_layer, date_buffer);
    text_layer_set_text(s_day_layer, day_buffer);
}

static void time_update_utc(void) {
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);

    static char utc_buffer[6];
    strftime(utc_buffer, sizeof(utc_buffer), "%H:%M", t);

    text_layer_set_text(s_utc_layer, utc_buffer);
}

void time_update_sunrise(time_t sunrise) {
    if (!s_sunrise_layer) {
        return;
    }

    if (sunrise == 0) {
        snprintf(s_sunrise_buffer, sizeof(s_sunrise_buffer), "--:--");
        text_layer_set_text(s_sunrise_layer, s_sunrise_buffer);
        return;
    }

    struct tm *t = localtime(&sunrise);
    format_time(s_sunrise_buffer, sizeof(s_sunrise_buffer), t);
    text_layer_set_text(s_sunrise_layer, s_sunrise_buffer);
}

void time_update_sunset(time_t sunset) {
    if (!s_sunset_layer) {
        return;
    }

    if (sunset == 0) {
        snprintf(s_sunset_buffer, sizeof(s_sunset_buffer), "--:--");
        text_layer_set_text(s_sunset_layer, s_sunset_buffer);
        return;
    }

    struct tm *t = localtime(&sunset);
    format_time(s_sunset_buffer, sizeof(s_sunset_buffer), t);
    text_layer_set_text(s_sunset_layer, s_sunset_buffer);
}

void time_update(void) {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[6];
    format_time(s_buffer, sizeof(s_buffer), tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
    time_update_ampm(tick_time);

    time_update_date_and_day();
    time_update_utc();
}

int border_radius = 4;

static void time_layer_container_update(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, THEME.bg_color_secondary);
    graphics_fill_rect(ctx, layer_get_bounds(layer), border_radius, GCornerTopLeft | GCornerTopRight);

    // Border separating main time from the date below.
    graphics_context_set_stroke_color(ctx, THEME.text_color_secondary);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, GPoint(bounds.origin.x, bounds.origin.y + bounds.size.h - 1),
                       GPoint(bounds.origin.x + bounds.size.w - 1, bounds.origin.y + bounds.size.h - 1));
}

static void date_layer_container_update(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 2);

    // Fill.
    graphics_context_set_fill_color(ctx, THEME.bg_color_secondary);
    graphics_fill_rect(ctx, layer_get_bounds(layer), border_radius, GCornersAll);
}

void time_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Lower container for date/day of week. Must be added before main time container.
    // It is the same height as the main time container, just shifted down a bit.
    int date_height = 18;
    s_date_layer_container = layer_create(GRect(0, (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2) + date_height,
                                                bounds.size.w, TIME_CONTAINER_HEIGHT));
    layer_set_update_proc(s_date_layer_container, date_layer_container_update);
    layer_add_child(window_layer, s_date_layer_container);

    // Date below main time display.
    GRect s_date_layer_container_frame = layer_get_frame(s_date_layer_container);
    int date_y = s_date_layer_container_frame.origin.y + s_date_layer_container_frame.size.h - date_height;
    s_date_layer = text_layer_create(GRect(0, date_y, bounds.size.w - PADDING_X, date_height));
    text_layer_set_font(s_date_layer, s_font_primary_small);
    text_layer_set_text_color(s_date_layer, THEME.text_color_secondary);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Day of the week.
    s_day_layer = text_layer_create(GRect(PADDING_X, date_y, bounds.size.w, date_height));
    text_layer_set_font(s_day_layer, s_font_primary_small);
    text_layer_set_text_color(s_day_layer, THEME.text_color_secondary);
    text_layer_set_background_color(s_day_layer, GColorClear);
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_day_layer));

    // Main time container.
    s_time_layer_container =
        layer_create(GRect(0, (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2), bounds.size.w, TIME_CONTAINER_HEIGHT));
    layer_set_update_proc(s_time_layer_container, time_layer_container_update);
    layer_add_child(window_layer, s_time_layer_container);

    // AM/PM indicator (12h mode only), top-left of the time container.
    s_ampm_layer = text_layer_create(GRect(PADDING_X, 0, bounds.size.w, 12));
    text_layer_set_font(s_ampm_layer, s_font_am_pm);
    text_layer_set_text_color(s_ampm_layer, THEME.text_color_secondary);
    text_layer_set_background_color(s_ampm_layer, GColorClear);
    text_layer_set_text_alignment(s_ampm_layer, GTextAlignmentLeft);
    layer_add_child(s_time_layer_container, text_layer_get_layer(s_ampm_layer));

    // Main time display.
    int time_height = 42;
    s_time_layer = text_layer_create(GRect(0, (TIME_CONTAINER_HEIGHT - time_height) / 2, bounds.size.w, time_height));
    text_layer_set_font(s_time_layer, s_font_time_large);
    text_layer_set_text_color(s_time_layer, THEME.text_color_secondary);
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
    layer_add_child(s_time_layer_container, text_layer_get_layer(s_time_layer));

    // Sunrise time.
    s_sunrise_icon_layer = font_render_icon_small(window_layer, ICON_SUNRISE, PADDING_X,
                                                  bounds.size.h - SOLAR_TIME_ROW_HEIGHT, false, false);
    text_layer_set_text_color(s_sunrise_icon_layer, THEME.text_color);
    GRect sunrise_icon_bounds = layer_get_bounds(text_layer_get_layer(s_sunrise_icon_layer));

    s_sunrise_layer = text_layer_create(
        GRect(PADDING_X + sunrise_icon_bounds.size.w + PADDING_X, bounds.size.h - SOLAR_TIME_ROW_HEIGHT,
              bounds.size.w - sunrise_icon_bounds.size.w - (PADDING_X * 2), SOLAR_TIME_ROW_HEIGHT));
    text_layer_set_font(s_sunrise_layer, s_font_primary_small);
    text_layer_set_text_color(s_sunrise_layer, THEME.text_color);
    text_layer_set_background_color(s_sunrise_layer, GColorClear);
    text_layer_set_text_alignment(s_sunrise_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_sunrise_layer));

    // Sunset time mirrored on the right.
    s_sunset_icon_layer = font_render_icon_small(window_layer, ICON_SUNSET, PADDING_X,
                                                 bounds.size.h - SOLAR_TIME_ROW_HEIGHT, true, false);
    text_layer_set_text_color(s_sunset_icon_layer, THEME.text_color);
    GRect sunset_icon_bounds = layer_get_bounds(text_layer_get_layer(s_sunset_icon_layer));

    int sunset_text_width = bounds.size.w - sunset_icon_bounds.size.w - (PADDING_X * 2);
    s_sunset_layer =
        text_layer_create(GRect(0, bounds.size.h - SOLAR_TIME_ROW_HEIGHT, sunset_text_width, SOLAR_TIME_ROW_HEIGHT));
    text_layer_set_font(s_sunset_layer, s_font_primary_small);
    text_layer_set_text_color(s_sunset_layer, THEME.text_color);
    text_layer_set_background_color(s_sunset_layer, GColorClear);
    text_layer_set_text_alignment(s_sunset_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(s_sunset_layer));

    // UTC time.
    int utc_y = bounds.size.h - SOLAR_TIME_ROW_HEIGHT - UTC_ROW_HEIGHT + 2;
    s_utc_icon_layer = font_render_icon_small(window_layer, ICON_UTC, PADDING_X, utc_y, false, false);
    text_layer_set_text_color(s_utc_icon_layer, THEME.text_color);
    GRect utc_icon_bounds = layer_get_bounds(text_layer_get_layer(s_utc_icon_layer));

    s_utc_layer = text_layer_create(GRect(PADDING_X + utc_icon_bounds.size.w + PADDING_X, utc_y,
                                          bounds.size.w - utc_icon_bounds.size.w - (PADDING_X * 2), UTC_ROW_HEIGHT));
    text_layer_set_font(s_utc_layer, s_font_primary_small);
    text_layer_set_text_color(s_utc_layer, THEME.text_color);
    text_layer_set_background_color(s_utc_layer, GColorClear);
    text_layer_set_text_alignment(s_utc_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_utc_layer));

    time_update();
    time_update_date_and_day();
    time_update_utc();
    time_update_sunrise(0);
    time_update_sunset(0);
}

void time_unload(void) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_ampm_layer);
    text_layer_destroy(s_utc_icon_layer);
    text_layer_destroy(s_utc_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_day_layer);
    text_layer_destroy(s_sunrise_icon_layer);
    text_layer_destroy(s_sunrise_layer);
    text_layer_destroy(s_sunset_icon_layer);
    text_layer_destroy(s_sunset_layer);
    layer_destroy(s_time_layer_container);
    layer_destroy(s_date_layer_container);
}
