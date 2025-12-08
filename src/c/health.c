#include "health.h"
#include "common.h"
#include "font.h"

TextLayer *s_steps_layer_text;
TextLayer *s_steps_layer_icon;

static void health_update(void) {
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
        health_update();
    }
}

void health_init() {
    health_service_events_subscribe(health_handler, NULL);
    health_update();
}

void health_load(Window *window, int row_height) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    int steps_y = row_height + 2;
    s_steps_layer_icon = font_render_icon_small(window_layer, ICON_STEPS, PADDING_X, steps_y, true, false);
    text_layer_set_text_color(s_steps_layer_icon, THEME.text_color);
    GRect steps_icon_bounds = layer_get_bounds(text_layer_get_layer(s_steps_layer_icon));

    s_steps_layer_text = text_layer_create(GRect(steps_icon_bounds.size.w - steps_icon_bounds.size.w, steps_y,
                                                 bounds.size.w - steps_icon_bounds.size.w - PADDING_X - 2, row_height));
    text_layer_set_text_alignment(s_steps_layer_text, GTextAlignmentRight);
    text_layer_set_font(s_steps_layer_text, s_font_primary_small);
    text_layer_set_text_color(s_steps_layer_text, THEME.text_color);
    text_layer_set_background_color(s_steps_layer_text, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_steps_layer_text));
}

void health_unload() {
    text_layer_destroy(s_steps_layer_icon);
    text_layer_destroy(s_steps_layer_text);
}
