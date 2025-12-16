#include "moon.h"
#include "common.h"
#include "font.h"
#include "time.h"

static TextLayer *s_moon_icon_layer;
static TextLayer *s_moon_layer;
static char s_moon_label_buffer[5] = "----";

static const char *MOON_ICONS[] = {"", "", "", "", "", "", "", "", "", "",
                                   "", "", "", "", "", "", "", "", "", "",
                                   "", "", "", "", "", "", "", ""};
static const char *MOON_LABELS[] = {"NEW",  "WXC1", "WXC2", "WXC3", "WXC4", "WXC5", "WXC6", "1ST",  "WXG1", "WXG2",
                                    "WXG3", "WXG4", "WXG5", "WXG6", "FULL", "WNG1", "WNG2", "WNG3", "WNG4", "WNG5",
                                    "WNG6", "3RD",  "WNC1", "WNC2", "WNC3", "WNC4", "WNC5", "WNC6"};
static const int MOON_PHASES = 28;
static const char *DEFAULT_MOON_ICON = "";

void moon_update(int32_t moon_phase) {
    if (!s_moon_layer || !s_moon_icon_layer) {
        return;
    }

    if (moon_phase < 0 || moon_phase >= MOON_PHASES) {
        text_layer_set_text(s_moon_layer, s_moon_label_buffer);
        text_layer_set_text(s_moon_icon_layer, DEFAULT_MOON_ICON);
        return;
    }

    snprintf(s_moon_label_buffer, sizeof(s_moon_label_buffer), "%s", MOON_LABELS[moon_phase]);
    text_layer_set_text(s_moon_layer, s_moon_label_buffer);
    text_layer_set_text(s_moon_icon_layer, MOON_ICONS[moon_phase]);
}

void moon_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    int moon_y = bounds.size.h - SOLAR_TIME_ROW_HEIGHT - UTC_ROW_HEIGHT + 2;

    // Icon on the right, above the sunset row.
    s_moon_icon_layer = font_render_icon_small(window_layer, DEFAULT_MOON_ICON, PADDING_X, moon_y, true, false);
    text_layer_set_text_color(s_moon_icon_layer, THEME.text_color);
    GRect moon_icon_bounds = layer_get_bounds(text_layer_get_layer(s_moon_icon_layer));

    // Label to the left of the icon, constrained to the right half to avoid overlapping UTC.
    int text_width = bounds.size.w - moon_icon_bounds.size.w - (PADDING_X * 2);
    s_moon_layer = text_layer_create(GRect(0, moon_y, text_width, UTC_ROW_HEIGHT));
    text_layer_set_font(s_moon_layer, s_font_primary_small);
    text_layer_set_text_color(s_moon_layer, THEME.text_color);
    text_layer_set_background_color(s_moon_layer, GColorClear);
    text_layer_set_text_alignment(s_moon_layer, GTextAlignmentRight);
    text_layer_set_text(s_moon_layer, s_moon_label_buffer);
    layer_add_child(window_layer, text_layer_get_layer(s_moon_layer));

    moon_update(-1);
}

void moon_unload(void) {
    text_layer_destroy(s_moon_icon_layer);
    text_layer_destroy(s_moon_layer);
    s_moon_icon_layer = NULL;
    s_moon_layer = NULL;
}
