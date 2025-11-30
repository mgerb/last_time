#include <pebble.h>

static Window *s_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer_text;
static TextLayer *s_battery_layer_icon;
static TextLayer *s_weather_layer_icon;
#if defined(PBL_HEALTH)
static TextLayer *s_steps_layer_text;
static TextLayer *s_steps_layer_icon;
#endif
static TextLayer *s_utc_icon_layer;
static TextLayer *s_utc_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;
static Layer *s_time_layer_container;

static GFont s_font_primary;
static GFont s_font_primary_small;
static GFont s_font_primary_bold;
static GFont s_font_icons;

static char *ICON_BATTERY_0 = "";
static char *ICON_BATTERY_25 = "";
static char *ICON_BATTERY_50 = "";
static char *ICON_BATTERY_75 = "";
static char *ICON_BATTERY_100 = "";
static char *ICON_CLOUDY = "\ue21d";
static char *ICON_STEPS = "";
static char *ICON_UTC = "";

static int PADDING = 2;

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

static void update_date_and_day(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    static char date_buffer[11];
    static char day_buffer[10];

    strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", t);
    strftime(day_buffer, sizeof(day_buffer), "%a", t);

    text_layer_set_text(s_date_layer, date_buffer);
    text_layer_set_text(s_day_layer, day_buffer);
}

static void update_utc_time(void) {
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);

    static char utc_buffer[6];
    strftime(utc_buffer, sizeof(utc_buffer), "%H:%M", t);

    text_layer_set_text(s_utc_layer, utc_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "tick handler...");
    update_time();
    update_date_and_day();
    update_utc_time();
}

static void update_battery_icon_and_text(BatteryChargeState state) {
    static char battery_buffer[8];
    snprintf(battery_buffer, sizeof(battery_buffer), "%d", state.charge_percent);
    text_layer_set_text(s_battery_layer_text, battery_buffer);

    // Pick icon based on charge percentage.
    char *icon = ICON_BATTERY_50;
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

static void battery_handler(BatteryChargeState state) {
    update_battery_icon_and_text(state);
}

#if defined(PBL_HEALTH)
static void update_steps(void) {
    static char steps_buffer[16];

    HealthServiceAccessibilityMask access =
        health_service_metric_accessible(HealthMetricStepCount, time_start_of_today(), time(NULL));
    if (access & HealthServiceAccessibilityMaskAvailable) {
        HealthValue steps = health_service_sum_today(HealthMetricStepCount);
        snprintf(steps_buffer, sizeof(steps_buffer), "%ld", (long)steps);
    } else {
        snprintf(steps_buffer, sizeof(steps_buffer), "--");
    }

    text_layer_set_text(s_steps_layer_text, steps_buffer);
}

static void health_handler(HealthEventType event, void *context) {
    if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
        update_steps();
    }
}
#endif

TextLayer *render_icon(Layer *container_layer, char *text, int x, int y, bool x_offset_right, bool y_offset_bottom) {
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

void load_fonts() {
    s_font_primary = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LILEX_MONO_45));
    // s_font_lilex_primary_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LILEX_MONO_14));
    s_font_primary_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PROGGY_CLEAN_MONO_18));
    s_font_primary_bold = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LILEX_MONO_45_BOLD));
    s_font_icons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ICONS_28));
}

void unload_fonts() {
    fonts_unload_custom_font(s_font_primary);
    fonts_unload_custom_font(s_font_primary_small);
    fonts_unload_custom_font(s_font_primary_bold);
    fonts_unload_custom_font(s_font_icons);
}

void load_top_bar(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int row_height = 14;

    // Battery icon.
    s_battery_layer_icon = render_icon(window_layer, ICON_BATTERY_50, PADDING, 0, true, false);

    // Battery percentage.
    GRect battery_icon_bounds = layer_get_bounds(text_layer_get_layer(s_battery_layer_icon));
    s_battery_layer_text = text_layer_create(GRect(battery_icon_bounds.size.w - battery_icon_bounds.size.w, 0,
                                                   bounds.size.w - battery_icon_bounds.size.w - 4, row_height));

    text_layer_set_text_alignment(s_battery_layer_text, GTextAlignmentRight);
    text_layer_set_font(s_battery_layer_text, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_background_color(s_battery_layer_text, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_battery_layer_text));
    update_battery_icon_and_text(battery_state_service_peek());

#if defined(PBL_HEALTH)
    int steps_y = row_height + PADDING;
    s_steps_layer_icon = render_icon(window_layer, ICON_STEPS, PADDING, steps_y, true, false);
    GRect steps_icon_bounds = layer_get_bounds(text_layer_get_layer(s_steps_layer_icon));

    s_steps_layer_text = text_layer_create(GRect(steps_icon_bounds.size.w - steps_icon_bounds.size.w, steps_y,
                                                 bounds.size.w - steps_icon_bounds.size.w - 4, row_height));
    text_layer_set_text_alignment(s_steps_layer_text, GTextAlignmentRight);
    text_layer_set_font(s_steps_layer_text, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_background_color(s_steps_layer_text, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_steps_layer_text));
#endif

    // Weather icon.
    s_weather_layer_icon = render_icon(window_layer, ICON_CLOUDY, PADDING, 0, false, false);
}

void load_time(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Main time display.
    s_time_layer_container = layer_create(GRect(0, (bounds.size.h / 2) - 30, bounds.size.w, 60));
    s_time_layer = text_layer_create(GRect(0, 0, layer_get_bounds(s_time_layer_container).size.w, 60));

    text_layer_set_font(s_time_layer, s_font_primary_bold);

    // For debugging only.
    text_layer_set_background_color(s_time_layer, GColorDarkGray);

    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    update_time();

    layer_add_child(window_layer, s_time_layer_container);
    layer_add_child(s_time_layer_container, text_layer_get_layer(s_time_layer));

    // Time in footer.
    int footer_height = 20;
    s_date_layer =
        text_layer_create(GRect(PADDING, bounds.size.h - footer_height - PADDING, bounds.size.w, footer_height));
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Day of the week.
    s_day_layer = text_layer_create(
        GRect(bounds.size.w / 2, bounds.size.h - footer_height - PADDING, bounds.size.w / 2 - PADDING, footer_height));
    text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_background_color(s_day_layer, GColorClear);
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(s_day_layer));

    // UTC time (above footer).
    int utc_height = 18;
    int utc_y = bounds.size.h - footer_height - utc_height + 2; // Space above date/day footer.
    s_utc_icon_layer = render_icon(window_layer, ICON_UTC, PADDING, utc_y, false, false);
    GRect utc_icon_bounds = layer_get_bounds(text_layer_get_layer(s_utc_icon_layer));

    s_utc_layer = text_layer_create(GRect(PADDING + utc_icon_bounds.size.w + PADDING, utc_y - 3,
                                          bounds.size.w - utc_icon_bounds.size.w - (PADDING * 2), utc_height));
    text_layer_set_font(s_utc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_background_color(s_utc_layer, GColorClear);
    text_layer_set_text_alignment(s_utc_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_utc_layer));

    update_date_and_day();
    update_utc_time();
}

static void window_load(Window *window) {
    // Load UI things.
    load_fonts();
    load_top_bar(window);
    load_time(window);

    // Subscribe to changes.
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_handler);
#if defined(PBL_HEALTH)
    health_service_events_subscribe(health_handler, NULL);
    update_steps();
#endif
}

static void window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_battery_layer_text);
    text_layer_destroy(s_battery_layer_icon);
#if defined(PBL_HEALTH)
    text_layer_destroy(s_steps_layer_icon);
    text_layer_destroy(s_steps_layer_text);
#endif
    text_layer_destroy(s_weather_layer_icon);
    text_layer_destroy(s_utc_icon_layer);
    text_layer_destroy(s_utc_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_day_layer);
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
#if defined(PBL_HEALTH)
    health_service_events_unsubscribe();
#endif
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
