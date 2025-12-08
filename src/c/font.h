#pragma once

#include <pebble.h>

extern GFont s_font_primary_small;
extern GFont s_font_primary_medium;

extern GFont s_font_time_small;
extern GFont s_font_time_large;

extern GFont s_font_icons_small;
extern GFont s_font_icons_medium;
extern GFont s_font_icons_large;

extern const char *ICON_BATTERY_0;
extern const char *ICON_BATTERY_25;
extern const char *ICON_BATTERY_50;
extern const char *ICON_BATTERY_75;
extern const char *ICON_BATTERY_100;
extern const char *ICON_STEPS;
extern const char *ICON_UTC;

void font_load(void);
void font_unload(void);
TextLayer *font_render_icon_small(Layer *container_layer, const char *text, int x, int y, bool x_offset_right,
                                  bool y_offset_bottom);
TextLayer *font_render_icon_medium(Layer *container_layer, const char *text, int x, int y, bool x_offset_right,
                                   bool y_offset_bottom);
TextLayer *font_render_icon_large(Layer *container_layer, const char *text, int x, int y, bool x_offset_right,
                                  bool y_offset_bottom);
