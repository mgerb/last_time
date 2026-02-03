#include "app_message.h"
#include "battery.h"
#include "bluetooth.h"
#include "common.h"
#include "font.h"
#include "log.h"
#include "moon.h"
#include "pebble.h"
#include "settings.h"
#include "time.h"
#include "weather.h"

#if defined(PBL_HEALTH)
#include "health.h"
#endif

static int s_last_vibrate_hour = -1;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    LOG_DEBUG("tick handler...");
    time_update();

    if (app_settings.vibrate_top_hour && tick_time->tm_min == 0 && tick_time->tm_hour != s_last_vibrate_hour) {
        vibes_short_pulse();
        s_last_vibrate_hour = tick_time->tm_hour;
    }

    weather_tick_handler();
}

void load_top_right(Window *window) {
    int row_height = 14;
    battery_load(window, row_height);
#if defined(PBL_HEALTH)
    if (app_settings.show_steps) {
        health_load(window, row_height);
    }
#endif
}

static void window_load(Window *window) {
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
    if (app_settings.show_steps) {
        health_unload();
    }
#endif
    font_unload();
}

static void init(void) {
    // Settings must be loaded before anything else, otherwise they
    // will contain garbage values, which will cause undefined behavior.
    settings_load();
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){.load = window_load, .unload = window_unload});
    const bool animated = true;
    window_stack_push(s_window, animated);

    // Subscribe to changes.
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_init();
    bluetooth_init();
#if defined(PBL_HEALTH)
    if (app_settings.show_steps) {
        health_init();
    }
#endif

    am_init();
}

static void deinit(void) {
    bluetooth_deinit();
    battery_deinit();
#if defined(PBL_HEALTH)
    if (app_settings.show_steps) {
        health_deinit();
    }
#endif
    tick_timer_service_unsubscribe();
    window_destroy(s_window);
}

int main(void) {
    init();

    LOG_DEBUG("Done "
              "initializing"
              ", pushed "
              "window: %p",
              s_window);

    app_event_loop();
    deinit();
}
