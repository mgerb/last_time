#include "app_message.h"
#include "common.h"
#include "font.h"
#include "pebble.h"
#include "time.h"
#include "weather.h"

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "tick handler...");
    time_update();
    time_update_date_and_day();
    time_update_utc();

    weather_request_if_needed();
}

/**
 * Update battery icon and text.
 */
static void battery_update_handler(BatteryChargeState state) {
    static char battery_buffer[8];
    snprintf(battery_buffer, sizeof(battery_buffer), "%d", state.charge_percent);
    text_layer_set_text(s_battery_layer_text, battery_buffer);

    // Pick icon based on charge percentage.
    char *icon = ICON_BATTERY_100;
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

void render_top_right_data(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int row_height = 14;

    // Battery icon.
    s_battery_layer_icon = font_render_icon(window_layer, ICON_BATTERY_50, PADDING_X, 0, true, false);
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

#if defined(PBL_HEALTH)
    int steps_y = row_height + 2;
    s_steps_layer_icon = font_render_icon(window_layer, ICON_STEPS, PADDING_X, steps_y, true, false);
    text_layer_set_text_color(s_steps_layer_icon, THEME.text_color);
    GRect steps_icon_bounds = layer_get_bounds(text_layer_get_layer(s_steps_layer_icon));

    s_steps_layer_text = text_layer_create(GRect(steps_icon_bounds.size.w - steps_icon_bounds.size.w, steps_y,
                                                 bounds.size.w - steps_icon_bounds.size.w - PADDING_X - 2, row_height));
    text_layer_set_text_alignment(s_steps_layer_text, GTextAlignmentRight);
    text_layer_set_font(s_steps_layer_text, s_font_primary_small);
    text_layer_set_text_color(s_steps_layer_text, THEME.text_color);
    text_layer_set_background_color(s_steps_layer_text, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_steps_layer_text));
#endif
}

static void window_load(Window *window) {
    // Load UI things.
    font_load();
    time_load(window);
    render_top_right_data(window);
    weather_load(window);

    window_set_background_color(window, THEME.bg_color);
}

static void window_unload(Window *window) {
    time_unload();
    weather_unload();
    text_layer_destroy(s_battery_layer_text);
    text_layer_destroy(s_battery_layer_icon);
#if defined(PBL_HEALTH)
    text_layer_destroy(s_steps_layer_icon);
    text_layer_destroy(s_steps_layer_text);
#endif
    font_unload();
}

static void init(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){.load = window_load, .unload = window_unload});
    const bool animated = true;
    window_stack_push(s_window, animated);

    // Subscribe to changes.
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_update_handler);
#if defined(PBL_HEALTH)
    health_service_events_subscribe(health_handler, NULL);
    update_steps();
#endif

    am_init();
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
