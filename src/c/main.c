#include "app_message.h"
#include "battery.h"
#include "bluetooth.h"
#include "common.h"
#include "font.h"
#include "moon.h"
#include "pebble.h"
#include "time.h"
#include "weather.h"

#if defined(PBL_HEALTH)
#include "health.h"
#endif

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "tick handler...");
    time_update();

    weather_request_if_needed();
    weather_update_condition_icon();
}

void load_top_right(Window *window) {
    int row_height = 14;
    battery_load(window, row_height);
#if defined(PBL_HEALTH)
    health_load(window, row_height);
#endif
}

static void window_load(Window *window) {
    // Load UI things.
    font_load();
    time_load(window);
    moon_load(window);
    load_top_right(window);
    bluetooth_load(window);
    weather_load(window);
    window_set_background_color(window, THEME.bg_color);
}

static void window_unload(Window *window) {
    time_unload();
    moon_unload();
    weather_unload();
    bluetooth_unload();
    battery_unload();
#if defined(PBL_HEALTH)
    health_unload();
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
    battery_init();
    bluetooth_init();
#if defined(PBL_HEALTH)
    health_init();
#endif

    am_init();
}

static void deinit(void) {
    bluetooth_deinit();
    battery_deinit();
#if defined(PBL_HEALTH)
    health_deinit();
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
