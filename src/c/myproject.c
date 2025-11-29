#include <pebble.h>

static Window *s_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer_text;
static TextLayer *s_battery_layer_icon;
static Layer *s_time_layer_container;

static GFont s_font_lilex_regular;
static GFont s_font_lilex_regular_small;
static GFont s_font_lilex_bold;
static GFont s_font_icons;

static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "tick handler...");
    update_time();
}

static void update_battery_text(BatteryChargeState state) {
    static char battery_buffer[8];
    snprintf(battery_buffer, sizeof(battery_buffer), "%d%%", state.charge_percent);
    text_layer_set_text(s_battery_layer_text, battery_buffer);
}

static void battery_handler(BatteryChargeState state) {
    update_battery_text(state);
}

void load_fonts() {
    s_font_lilex_regular = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LILEX_MONO_45));
    s_font_lilex_regular_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LILEX_MONO_14));
    s_font_lilex_bold = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LILEX_MONO_45_BOLD));
    s_font_icons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ICONS_28));
}

void unload_fonts() {
    fonts_unload_custom_font(s_font_lilex_regular);
    fonts_unload_custom_font(s_font_lilex_regular_small);
    fonts_unload_custom_font(s_font_lilex_bold);
    fonts_unload_custom_font(s_font_icons);
}

void load_top_bar(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Battery icon.
    s_battery_layer_icon = text_layer_create(GRect(0, -8, bounds.size.w - 4, 35));
    text_layer_set_text_alignment(s_battery_layer_icon, GTextAlignmentRight);
    text_layer_set_font(s_battery_layer_icon, s_font_icons);
    text_layer_set_text(s_battery_layer_icon, "\xEF\x89\x82");
    layer_add_child(window_layer, text_layer_get_layer(s_battery_layer_icon));

    // Battery percentage.
    s_battery_layer_text = text_layer_create(GRect(0, 0, bounds.size.w - 25, 35));
    text_layer_set_text_alignment(s_battery_layer_text, GTextAlignmentRight);
    text_layer_set_font(s_battery_layer_text, s_font_lilex_regular_small);
    update_battery_text(battery_state_service_peek());
    layer_add_child(window_layer, text_layer_get_layer(s_battery_layer_text));
}

void load_time(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_time_layer_container = layer_create(GRect(0, (bounds.size.h / 2) - 30, bounds.size.w, 60));

    s_time_layer = text_layer_create(GRect(0, 0, layer_get_bounds(s_time_layer_container).size.w, 60));

    text_layer_set_font(s_time_layer, s_font_lilex_bold);
    // For debugging only.
    text_layer_set_background_color(s_time_layer, GColorDarkGray);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    update_time();

    layer_add_child(window_layer, s_time_layer_container);
    layer_add_child(s_time_layer_container, text_layer_get_layer(s_time_layer));
}

static void window_load(Window *window) {
    // Load UI things.
    load_fonts();
    load_top_bar(window);
    load_time(window);

    // Subscribe to changes.
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_handler);
}

static void window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_battery_layer_text);
    layer_destroy(s_time_layer_container);
    unload_fonts();
}

static void init(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){.load = window_load, .unload = window_unload});
    const bool animated = true;
    window_stack_push(s_window, animated);
}

static void deinit(void) {
    battery_state_service_unsubscribe();
    window_destroy(s_window);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG,
            "Done "
            "initializing"
            ", pushed "
            "window: %p",
            s_window);

    app_event_loop();
    deinit();
}
