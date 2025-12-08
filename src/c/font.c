#include "font.h"

GFont s_font_primary_small;
GFont s_font_primary_medium;

GFont s_font_time_small;
GFont s_font_time_large;

GFont s_font_icons_xsmall;
GFont s_font_icons_small;
GFont s_font_icons_medium;
GFont s_font_icons_large;

const char *ICON_BATTERY_0 = "";
const char *ICON_BATTERY_25 = "";
const char *ICON_BATTERY_50 = "";
const char *ICON_BATTERY_75 = "";
const char *ICON_BATTERY_100 = "";
const char *ICON_STEPS = "";
const char *ICON_UTC = "";
const char *ICON_BLUETOOTH_CONNECTED = "󰂯";
const char *ICON_BLUETOOTH_DISCONNECTED = "󰂲";

static TextLayer *font_render_icon(Layer *container_layer, GFont font, const char *text, int x, int y,
                                   bool x_offset_right, bool y_offset_bottom) {
    GRect bounds = layer_get_bounds(container_layer);
    TextLayer *icon_layer = text_layer_create(GRect(x, y, 100, 100));
    text_layer_set_font(icon_layer, font);
    text_layer_set_text(icon_layer, text);
    GSize content_size = text_layer_get_content_size(icon_layer);
    text_layer_set_size(icon_layer, content_size);
    text_layer_set_background_color(icon_layer, GColorClear);

    int new_x = x;
    int new_y = y;

    if (x_offset_right) {
        new_x = bounds.size.w - content_size.w - x;
    }

    if (y_offset_bottom) {
        new_y = bounds.size.h - content_size.h - y;
    }

    // Subtract 8 from the height, because there always seems to be extra height. Maybe it's from
    // the font's line height? Not quite sure.
    layer_set_frame(text_layer_get_layer(icon_layer), GRect(new_x, new_y - 8, content_size.w, content_size.h));

    layer_add_child(container_layer, text_layer_get_layer(icon_layer));
    return icon_layer;
}

void font_load(void) {
    s_font_primary_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINUS_MONO_14));
    s_font_primary_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINUS_MONO_20));

    // Time font.
    s_font_time_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DSEG_12));
    s_font_time_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DSEG_42));

    // Nerd font icons. See fonts in package.json. Using Lilex nerd fonts because
    // icons just did not render right with other nerd fonts for some reason.
    s_font_icons_xsmall = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ICONS_21));
    s_font_icons_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ICONS_28));
    s_font_icons_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ICONS_36));
    s_font_icons_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ICONS_48));
}

void font_unload(void) {
    // Primary fonts.
    fonts_unload_custom_font(s_font_primary_small);
    fonts_unload_custom_font(s_font_primary_medium);

    // Time font.
    fonts_unload_custom_font(s_font_time_small);
    fonts_unload_custom_font(s_font_time_large);

    // Icons.
    fonts_unload_custom_font(s_font_icons_xsmall);
    fonts_unload_custom_font(s_font_icons_small);
    fonts_unload_custom_font(s_font_icons_medium);
    fonts_unload_custom_font(s_font_icons_large);
}

TextLayer *font_render_icon_xsmall(Layer *container_layer, const char *text, int x, int y, bool x_offset_right,
                                   bool y_offset_bottom) {
    return font_render_icon(container_layer, s_font_icons_xsmall, text, x, y, x_offset_right, y_offset_bottom);
}

TextLayer *font_render_icon_small(Layer *container_layer, const char *text, int x, int y, bool x_offset_right,
                                  bool y_offset_bottom) {
    return font_render_icon(container_layer, s_font_icons_small, text, x, y, x_offset_right, y_offset_bottom);
}

TextLayer *font_render_icon_medium(Layer *container_layer, const char *text, int x, int y, bool x_offset_right,
                                   bool y_offset_bottom) {
    return font_render_icon(container_layer, s_font_icons_medium, text, x, y, x_offset_right, y_offset_bottom);
}

TextLayer *font_render_icon_large(Layer *container_layer, const char *text, int x, int y, bool x_offset_right,
                                  bool y_offset_bottom) {
    return font_render_icon(container_layer, s_font_icons_large, text, x, y, x_offset_right, y_offset_bottom);
}
