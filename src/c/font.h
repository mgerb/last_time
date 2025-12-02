#pragma once

#include "common.h"

static GFont s_font_primary;
static GFont s_font_primary_small;
static GFont s_font_primary_bold;
static GFont s_font_icons;

static char *ICON_BATTERY_0 = "";
static char *ICON_BATTERY_25 = "";
static char *ICON_BATTERY_50 = "";
static char *ICON_BATTERY_75 = "";
static char *ICON_BATTERY_100 = "";
static char *ICON_CLOUDY = "\ue21d"; // TODO:
static char *ICON_STEPS = "";
static char *ICON_UTC = "";

void font_load() {
    // s_font_primary = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINUS_MONO_48));
    // s_font_lilex_primary_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LILEX_MONO_14));
    s_font_primary_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINUS_MONO_14));
    s_font_primary_bold = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DSEG_42));
    // s_font_primary_bold = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINUS_MONO_48));
    s_font_icons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ICONS_28));
}

void font_unload() {
    // fonts_unload_custom_font(s_font_primary);
    fonts_unload_custom_font(s_font_primary_small);
    fonts_unload_custom_font(s_font_primary_bold);
    fonts_unload_custom_font(s_font_icons);
}

TextLayer *font_render_icon(Layer *container_layer, char *text, int x, int y, bool x_offset_right, bool y_offset_bottom) {
    GRect bounds = layer_get_bounds(container_layer);
    TextLayer *icon_layer = text_layer_create(GRect(x, y, 100, 100));
    text_layer_set_font(icon_layer, s_font_icons);
    text_layer_set_text(icon_layer, text);
    GSize content_size = text_layer_get_content_size(icon_layer);
    text_layer_set_size(icon_layer, content_size);
    text_layer_set_background_color(icon_layer, GColorClear);

    // NOTE: Debug only.
    // text_layer_set_background_color(icon_layer, GColorDarkGray);

    int new_x = x;
    int new_y = y;

    if (x_offset_right) {
        new_x = bounds.size.w - content_size.w - x;
    }

    if (y_offset_bottom) {
        new_y = bounds.size.h - content_size.h - y;
    }

    // Subtract 8 from the height, because there always seems to be extra height. Maybe it's from
    // the font's line height? Note quite sure.
    layer_set_frame(text_layer_get_layer(icon_layer), GRect(new_x, new_y - 8, content_size.w, content_size.h));

    layer_add_child(container_layer, text_layer_get_layer(icon_layer));
    return icon_layer;
}
