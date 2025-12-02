#pragma once

#include "common.h"
#include "font.h"
#include "gcolor_definitions.h"
#include "pebble.h"

static TextLayer *s_time_layer;
static Layer *s_time_layer_container;
static TextLayer *s_utc_icon_layer;
static TextLayer *s_utc_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;

static const int TIME_CONTAINER_HEIGHT = 56;

static void time_update() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void time_update_date_and_day(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    static char date_buffer[11];
    static char day_buffer[10];

    strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", t);
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

static void time_container_update_color(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_stroke_color(ctx, THEME.bg_color);
    graphics_context_set_stroke_width(ctx, 2);

    // Top border.
    graphics_draw_line(ctx, GPoint(bounds.origin.x, bounds.origin.y),
                       GPoint(bounds.origin.x + bounds.size.w - 1, bounds.origin.y));

    // Bottom border.
    graphics_draw_line(ctx, GPoint(bounds.origin.x, bounds.origin.y + bounds.size.h - 1),
                       GPoint(bounds.origin.x + bounds.size.w - 1, bounds.origin.y + bounds.size.h - 1));

    // Fill.
    graphics_context_set_fill_color(ctx, THEME.bg_color_secondary);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);
}

static void time_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Main time container.
    s_time_layer_container =
        layer_create(GRect(0, (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2), bounds.size.w, TIME_CONTAINER_HEIGHT));
    layer_set_update_proc(s_time_layer_container, time_container_update_color);
    layer_add_child(window_layer, s_time_layer_container);

    // Main time display.
    int time_height = 42;
    s_time_layer = text_layer_create(GRect(0, (TIME_CONTAINER_HEIGHT - time_height) / 2, bounds.size.w, time_height));
    text_layer_set_font(s_time_layer, s_font_primary_bold);
    text_layer_set_text_color(s_time_layer, THEME.text_color_secondary);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(s_time_layer_container, text_layer_get_layer(s_time_layer));

    // Date in footer.
    int footer_height = 20;
    s_date_layer = text_layer_create(GRect(PADDING_X, bounds.size.h - footer_height, bounds.size.w, footer_height));
    text_layer_set_font(s_date_layer, s_font_primary_small);
    text_layer_set_text_color(s_date_layer, THEME.text_color);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Day of the week.
    s_day_layer = text_layer_create(
        GRect(bounds.size.w / 2, bounds.size.h - footer_height, bounds.size.w / 2 - PADDING_X, footer_height));
    text_layer_set_font(s_day_layer, s_font_primary_small);
    text_layer_set_text_color(s_day_layer, THEME.text_color);
    text_layer_set_background_color(s_day_layer, GColorClear);
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(s_day_layer));

    // UTC time (above footer).
    int utc_height = 18;
    int utc_y = bounds.size.h - footer_height - utc_height + 2; // Space above date/day footer.
    s_utc_icon_layer = font_render_icon(window_layer, ICON_UTC, PADDING_X, utc_y, false, false);
    text_layer_set_text_color(s_utc_icon_layer, THEME.text_color);
    GRect utc_icon_bounds = layer_get_bounds(text_layer_get_layer(s_utc_icon_layer));

    s_utc_layer = text_layer_create(GRect(PADDING_X + utc_icon_bounds.size.w + PADDING_X, utc_y,
                                          bounds.size.w - utc_icon_bounds.size.w - (PADDING_X * 2), utc_height));
    text_layer_set_font(s_utc_layer, s_font_primary_small);
    text_layer_set_text_color(s_utc_layer, THEME.text_color);
    text_layer_set_background_color(s_utc_layer, GColorClear);
    text_layer_set_text_alignment(s_utc_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_utc_layer));

    time_update();
    time_update_date_and_day();
    time_update_utc();
}

static void time_unload() {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_utc_icon_layer);
    text_layer_destroy(s_utc_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_day_layer);
    layer_destroy(s_time_layer_container);
}
