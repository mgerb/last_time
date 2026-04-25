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

#if defined(PBL_PLATFORM_EMERY)
static const int HEART_RATE_Y_OFFSET = 9;
#else
static const int HEART_RATE_Y_OFFSET = 0;
#endif
#endif

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

void health_init() {
    health_service_events_subscribe(health_handler, NULL);
    health_update();
}

void health_load(Window *window, int row_height) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // --- Steps ---

    if (app_settings.show_steps) {
        int steps_y = row_height + 2;
        s_steps_layer_icon = font_render_icon_small(window_layer, ICON_STEPS, PADDING_X, steps_y, true, false);
        text_layer_set_text_color(s_steps_layer_icon, THEME.text_color);
        GRect steps_icon_bounds = layer_get_bounds(text_layer_get_layer(s_steps_layer_icon));

        s_steps_layer_text =
            text_layer_create(GRect(0, steps_y, bounds.size.w - steps_icon_bounds.size.w - PADDING_X - 2, row_height));
        text_layer_set_text_alignment(s_steps_layer_text, GTextAlignmentRight);
        text_layer_set_font(s_steps_layer_text, s_font_primary_small);
        text_layer_set_text_color(s_steps_layer_text, THEME.text_color);
        text_layer_set_background_color(s_steps_layer_text, GColorClear);
        layer_add_child(window_layer, text_layer_get_layer(s_steps_layer_text));
    }

    // --- Heart Rate ---

#if defined(HEART_RATE_SUPPORTED)
#if defined(PBL_PLATFORM_EMERY)
    int heart_rate_y = (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2) - 20 - HEART_RATE_Y_OFFSET;
#else
    int heart_rate_y = (bounds.size.h / 2) - (TIME_CONTAINER_HEIGHT / 2) - 14 - HEART_RATE_Y_OFFSET;
#endif
    s_heart_rate_layer_icon =
        font_render_icon_small(window_layer, ICON_HEART_RATE, PADDING_X, heart_rate_y, true, false);
    text_layer_set_text_color(s_heart_rate_layer_icon, THEME.text_color);
    GRect heart_rate_icon_bounds = layer_get_bounds(text_layer_get_layer(s_heart_rate_layer_icon));

    s_heart_rate_layer_text = text_layer_create(
        GRect(0, heart_rate_y, bounds.size.w - heart_rate_icon_bounds.size.w - PADDING_X - 2, row_height));
    text_layer_set_text_alignment(s_heart_rate_layer_text, GTextAlignmentRight);
    text_layer_set_font(s_heart_rate_layer_text, s_font_primary_small);
    text_layer_set_text_color(s_heart_rate_layer_text, THEME.text_color);
    text_layer_set_background_color(s_heart_rate_layer_text, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_heart_rate_layer_text));
#endif

    health_update();
}

void health_unload() {
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
}

void health_deinit() {
    health_service_events_unsubscribe();
}
