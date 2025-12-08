#include "battery.h"
#include "common.h"
#include "font.h"

static TextLayer *s_battery_layer_text;
static TextLayer *s_battery_layer_icon;

/**
 * Update battery icon and text.
 */
static void battery_update_handler(BatteryChargeState state) {
    static char battery_buffer[8];
    snprintf(battery_buffer, sizeof(battery_buffer), "%d", state.charge_percent);
    text_layer_set_text(s_battery_layer_text, battery_buffer);

    // Pick icon based on charge percentage.
    const char *icon = ICON_BATTERY_100;
    if (state.charge_percent <= 5) {
        icon = ICON_BATTERY_0;
    } else if (state.charge_percent <= 30) {
        icon = ICON_BATTERY_25;
    } else if (state.charge_percent <= 60) {
        icon = ICON_BATTERY_50;
    } else if (state.charge_percent <= 85) {
        icon = ICON_BATTERY_75;
    } else {
        icon = ICON_BATTERY_100;
    }

    text_layer_set_text(s_battery_layer_icon, icon);
}

void battery_init() {
    battery_state_service_subscribe(battery_update_handler);
}

void battery_load(Window *window, int row_height) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Battery icon.
    s_battery_layer_icon = font_render_icon_small(window_layer, ICON_BATTERY_50, PADDING_X, 0, true, false);
    text_layer_set_text_color(s_battery_layer_icon, THEME.text_color);

    // Battery percentage.
    GRect battery_icon_bounds = layer_get_bounds(text_layer_get_layer(s_battery_layer_icon));
    s_battery_layer_text =
        text_layer_create(GRect(battery_icon_bounds.size.w - battery_icon_bounds.size.w, 0,
                                bounds.size.w - battery_icon_bounds.size.w - PADDING_X - 2, row_height));

    text_layer_set_text_alignment(s_battery_layer_text, GTextAlignmentRight);
    text_layer_set_font(s_battery_layer_text, s_font_primary_small);
    text_layer_set_text_color(s_battery_layer_text, THEME.text_color);
    text_layer_set_background_color(s_battery_layer_text, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_battery_layer_text));
    battery_update_handler(battery_state_service_peek());
}

void battery_unload() {
    text_layer_destroy(s_battery_layer_text);
    text_layer_destroy(s_battery_layer_icon);
}
