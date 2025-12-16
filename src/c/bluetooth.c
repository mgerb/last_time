#include "bluetooth.h"
#include "pebble.h"
#include "time.h"

static TextLayer *s_bluetooth_layer_icon;
static bool s_bluetooth_connected = false;

static void bluetooth_update_icon(void) {
    if (!s_bluetooth_layer_icon) {
        return;
    }

    layer_set_hidden(text_layer_get_layer(s_bluetooth_layer_icon), !s_bluetooth_connected);
    text_layer_set_text_color(s_bluetooth_layer_icon, THEME.text_color);
    text_layer_set_text(s_bluetooth_layer_icon, ICON_BLUETOOTH_CONNECTED);
}

static void bluetooth_connection_handler(bool connected) {
    // Vibrate on connect/disconnect.
    if (connected != s_bluetooth_connected) {
        vibes_short_pulse();
    }

    s_bluetooth_connected = connected;
    bluetooth_update_icon();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_handler: %d", connected);
}

void bluetooth_refresh_connected_state(void) {

    s_bluetooth_connected = connection_service_peek_pebble_app_connection();
}

void bluetooth_init(void) {
    bluetooth_refresh_connected_state();
    connection_service_subscribe((ConnectionHandlers){.pebble_app_connection_handler = bluetooth_connection_handler});
}

void bluetooth_deinit(void) {
    connection_service_unsubscribe();
}

void bluetooth_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Position icon on the right, just above the time container.
    int icon_y = (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2) - 14;
    s_bluetooth_layer_icon =
        font_render_icon_xsmall(window_layer, ICON_BLUETOOTH_CONNECTED, PADDING_X, icon_y, true, false);
    text_layer_set_text_color(s_bluetooth_layer_icon, THEME.text_color);

    // Refresh state in case the initial peek happened before the service was ready.
    bluetooth_refresh_connected_state();
    bluetooth_update_icon();
}

void bluetooth_unload(void) {
    text_layer_destroy(s_bluetooth_layer_icon);
    s_bluetooth_layer_icon = NULL;
}
