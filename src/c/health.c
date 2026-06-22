#include "health.h"
#include "common.h"
#include "font.h"
#include "settings.h"
#include "time.h"

static TextLayer *s_steps_layer_text;
static TextLayer *s_steps_layer_icon;

#if defined(HEART_RATE_SUPPORTED)
static TextLayer *s_heart_rate_layer_text;
static TextLayer *s_heart_rate_layer_icon;
#endif

static bool s_health_initialized = false;
static bool s_health_loaded = false;

static bool health_should_run(void) {
#if defined(HEART_RATE_SUPPORTED)
    return app_settings.show_steps || app_settings.show_heart_rate;
#else
    return app_settings.show_steps;
#endif
}

static void health_update_steps(void) {
    if (!s_steps_layer_text) {
        return;
    }

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

static void health_update_heart_rate(void) {
#if defined(HEART_RATE_SUPPORTED)
    if (!s_heart_rate_layer_text) {
        return;
    }

    static char heart_rate_buffer[16];

    HealthServiceAccessibilityMask access = health_service_metric_aggregate_averaged_accessible(
        HealthMetricHeartRateBPM, time(NULL), time(NULL), HealthAggregationAvg, HealthServiceTimeScopeOnce);
    if (access & HealthServiceAccessibilityMaskAvailable) {
        HealthValue heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);
        if (heart_rate > 0) {
            snprintf(heart_rate_buffer, sizeof(heart_rate_buffer), "%ld", (long)heart_rate);
        } else {
            snprintf(heart_rate_buffer, sizeof(heart_rate_buffer), "--");
        }
    } else {
        snprintf(heart_rate_buffer, sizeof(heart_rate_buffer), "--");
    }

    text_layer_set_text(s_heart_rate_layer_text, heart_rate_buffer);
#endif
}

static void health_update(void) {
    health_update_steps();
    health_update_heart_rate();
}

static void health_handler(HealthEventType event, void *context) {
    if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
        health_update_steps();
    }

    if (event == HealthEventHeartRateUpdate || event == HealthEventSignificantUpdate) {
        health_update_heart_rate();
    }
}

// Initialize health services.
static void health_init(void) {
    if (s_health_initialized) {
        return;
    }

    health_service_events_subscribe(health_handler, NULL);
    s_health_initialized = true;
    health_update();
}

// Load health UI.
void health_load(Window *window, int row_height) {
    if (!health_should_run()) {
        health_unload();
        return;
    }

    health_unload();

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // --- Steps ---

    if (app_settings.show_steps) {
        int steps_y = row_height + 2;
        s_steps_layer_icon = font_render_icon_small(window_layer, ICON_STEPS, PADDING_X, steps_y, true, false);
        text_layer_set_text_color(s_steps_layer_icon, app_settings.text_color);
        GRect steps_icon_bounds = layer_get_bounds(text_layer_get_layer(s_steps_layer_icon));

        s_steps_layer_text =
            text_layer_create(GRect(0, steps_y, bounds.size.w - steps_icon_bounds.size.w - PADDING_X - 2, row_height));
        text_layer_set_text_alignment(s_steps_layer_text, GTextAlignmentRight);
        text_layer_set_font(s_steps_layer_text, s_font_primary_small);
        text_layer_set_text_color(s_steps_layer_text, app_settings.text_color);
        text_layer_set_background_color(s_steps_layer_text, GColorClear);
        layer_add_child(window_layer, text_layer_get_layer(s_steps_layer_text));
    }

    // --- Heart Rate ---

#if defined(HEART_RATE_SUPPORTED)
#if defined(PBL_PLATFORM_EMERY)
    int heart_rate_y = (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2) - 30;
#else
    int heart_rate_y = (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2) - 21;
#endif
    if (app_settings.show_heart_rate) {
        s_heart_rate_layer_icon =
            font_render_icon_small(window_layer, ICON_HEART_RATE, PADDING_X, heart_rate_y, true, false);
        text_layer_set_text_color(s_heart_rate_layer_icon, app_settings.text_color);
        GRect heart_rate_icon_bounds = layer_get_bounds(text_layer_get_layer(s_heart_rate_layer_icon));

        s_heart_rate_layer_text = text_layer_create(
            GRect(0, heart_rate_y, bounds.size.w - heart_rate_icon_bounds.size.w - PADDING_X - 2, row_height));
        text_layer_set_text_alignment(s_heart_rate_layer_text, GTextAlignmentRight);
        text_layer_set_font(s_heart_rate_layer_text, s_font_primary_small);
        text_layer_set_text_color(s_heart_rate_layer_text, app_settings.text_color);
        text_layer_set_background_color(s_heart_rate_layer_text, GColorClear);
        layer_add_child(window_layer, text_layer_get_layer(s_heart_rate_layer_text));
    }
#endif

    s_health_loaded = true;
    health_update();
}

void health_unload(void) {
    if (!s_health_loaded) {
        return;
    }

    if (s_steps_layer_icon) {
        text_layer_destroy(s_steps_layer_icon);
        s_steps_layer_icon = NULL;
    }

    if (s_steps_layer_text) {
        text_layer_destroy(s_steps_layer_text);
        s_steps_layer_text = NULL;
    }
#if defined(HEART_RATE_SUPPORTED)
    if (s_heart_rate_layer_icon) {
        text_layer_destroy(s_heart_rate_layer_icon);
        s_heart_rate_layer_icon = NULL;
    }

    if (s_heart_rate_layer_text) {
        text_layer_destroy(s_heart_rate_layer_text);
        s_heart_rate_layer_text = NULL;
    }
#endif

    s_health_loaded = false;
}

// Init or deinit health services based on app settings.
void health_sync_service(void) {
    if (health_should_run()) {
        health_init();
    } else {
        health_deinit();
    }
}

void health_deinit(void) {
    if (!s_health_initialized) {
        return;
    }

    health_service_events_unsubscribe();
    s_health_initialized = false;
}
