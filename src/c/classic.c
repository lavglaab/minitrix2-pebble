#include <pebble.h>
#include "utils.h"
#include "classic.h"
#include "debug_flags.h"

static Layer *s_layer_background;
static TextLayer *s_layer_date;
static TextLayer *s_layer_weather;

static GFont classic_font_time;

static GDrawCommandImage *s_pdc_classic_carets;
static GDrawCommandImage *s_pdc_classic_jewel;
static GDrawCommandImage *s_pdc_classic_jewel_stroke;

static GColor classic_color_background;

static bool classic_time_showing = false;

/* ---------- UI ----------*/
static GColor prv_classic_jewel_color() {
  GColor color = PAL_CLASSIC_JEWEL;
  #if defined(PBL_COLOR)
  if (s_settings.DoColorOverride) { color = s_settings.CustomColor; }
  if (battery_state_service_peek().charge_percent <= 10) { color = PAL_CLASSIC_STATUS_LOWBATT; }
  if (!connection_service_peek_pebble_app_connection()) { color = PAL_CLASSIC_STATUS_DISCONNECT; }
  #endif
  return color;
}

static GColor prv_classic_complication_color() {
  GColor color = PAL_CLASSIC_COMPLICATIONS;
  #if defined(PBL_COLOR)

  if (s_settings.HighContrast) {
    color = gcolor_legible_over(prv_classic_jewel_color());
    return color;
  }

  if (s_settings.DoColorOverride ) { color = gcolor_legible_over(s_settings.CustomColor); }
  if (battery_state_service_peek().charge_percent <= 10) { color = PAL_CLASSIC_TEXT_LOWBATT; }
  if (!connection_service_peek_pebble_app_connection()) { color = PAL_CLASSIC_TEXT_DISCONNECT; }
  #endif
  return color;
}

void classic_update_minute() {
  layer_mark_dirty(s_layer_background);
}

void classic_update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char buffer_date[16];
  strftime(buffer_date, sizeof(buffer_date), PBL_IF_RECT_ELSE("%a %b%e", "%a%n%b%e"), tick_time);
  text_layer_set_text(s_layer_date, buffer_date);
}

void classic_update_weather(int response_code) {
  // set textlayer and icon
}

void classic_update_style() {
  text_layer_set_text_color(s_layer_date, prv_classic_complication_color());
  if (s_layer_weather) { text_layer_set_text_color(s_layer_weather, prv_classic_complication_color()); }
  layer_mark_dirty(s_layer_background);
}

void classic_ui_set_hidden(bool value) {
  classic_time_showing = !value;
  layer_set_hidden(text_layer_get_layer(s_layer_date), value);
  if (s_settings.DoWeather) { layer_set_hidden(text_layer_get_layer(s_layer_weather), value); }
  layer_mark_dirty(s_layer_background);
}

/* ---------- Update procs ---------- */
static void update_proc_classic_bg(Layer *layer, GContext *ctx) {
    LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "update_proc_classic_bg");
    GRect bounds = layer_get_bounds(layer);
    GPoint origin = GPoint(0, 0);

    if (!s_pdc_classic_carets) { s_pdc_classic_carets = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_CARETS); }
    if (!s_pdc_classic_jewel) { s_pdc_classic_jewel = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_JEWEL); }
    if (!s_pdc_classic_jewel_stroke) { s_pdc_classic_jewel_stroke = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_JEWEL_STROKE); }
    
    //Clear canvas
    graphics_context_set_fill_color(ctx, classic_color_background);
    graphics_fill_rect(ctx, bounds, 0, GCornersAll);


    //Black carets
    gdraw_command_image_draw(ctx, s_pdc_classic_carets, origin);

    // Draw time
    if (!s_settings.HideUI || classic_time_showing) {
    if (!classic_font_time) {
      classic_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BALOO_60));
    }
    graphics_context_set_text_color(ctx, PAL_CLASSIC_CLOCK);
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    char classic_time_hour[8];
    char classic_time_minute[8];
    strftime(classic_time_hour, sizeof(classic_time_hour), clock_is_24h_style() ?
                                        "%H" : "%I", tick_time);
    strftime(classic_time_minute, sizeof(classic_time_minute), "%M", tick_time);

    LOG_IF_ENABLED(DEBUG_LOG_TIMESTRINGS, APP_LOG_LEVEL_DEBUG, "time_hour: %s", classic_time_hour);
    LOG_IF_ENABLED(DEBUG_LOG_TIMESTRINGS, APP_LOG_LEVEL_DEBUG, "time_minute: %s", classic_time_minute);

    #if defined(PBL_RECT)
    static char ch1[2];
    static char ch2[2];
    static char cm1[2];
    static char cm2[2];

    strncpy(ch1, classic_time_hour+0, 1);
    strncpy(ch2, classic_time_hour+1, 1);
    strncpy(cm1, classic_time_minute+0, 1);
    strncpy(cm2, classic_time_minute+1, 1);
    
    graphics_draw_text(ctx, ch1, classic_font_time, BOUND_CLASSIC_TIME_H1, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0); //Hour1
    graphics_draw_text(ctx, ch2, classic_font_time, BOUND_CLASSIC_TIME_H2, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0); //Hour2
    graphics_draw_text(ctx, cm1, classic_font_time, BOUND_CLASSIC_TIME_M1, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0); //Minute1
    graphics_draw_text(ctx, cm2, classic_font_time, BOUND_CLASSIC_TIME_M2, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0); //Minute2
    
    
    LOG_IF_ENABLED(DEBUG_LOG_TIMESTRINGS, APP_LOG_LEVEL_DEBUG, "h1: %s", ch1);
    LOG_IF_ENABLED(DEBUG_LOG_TIMESTRINGS, APP_LOG_LEVEL_DEBUG, "h2: %s", ch2);
    LOG_IF_ENABLED(DEBUG_LOG_TIMESTRINGS, APP_LOG_LEVEL_DEBUG, "m1: %s", cm1);
    LOG_IF_ENABLED(DEBUG_LOG_TIMESTRINGS, APP_LOG_LEVEL_DEBUG, "m2: %s", cm2);
    #endif

    #if defined(PBL_ROUND)
    graphics_draw_text(ctx, classic_time_hour, classic_font_time, BOUND_CLASSIC_TIME_HOUR, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0); //Hour
    graphics_draw_text(ctx, classic_time_minute, classic_font_time, BOUND_CLASSIC_TIME_MINUTE, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0); //Minute
    #endif
    }

    //Jewel
    gdraw_command_image_draw(ctx, s_pdc_classic_jewel, origin);
    
    //Jewel stroke
    gdraw_command_image_draw(ctx, s_pdc_classic_jewel_stroke, origin);
}

/* ---------- Life cycle ----------*/

void classic_window_load(Window *window) {
  //Set background color according to classic or uaf
  classic_color_background = (s_settings.DialMode == 'c') 
    ? PAL_CLASSIC_GREY_CARETS : PAL_CLASSIC_BLACK_CARETS;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //Render layer with time
  s_layer_background = layer_create(bounds);
  layer_set_update_proc(s_layer_background, update_proc_classic_bg);
  layer_add_child(window_layer, s_layer_background);

  //Date layer
  s_layer_date = text_layer_create(BOUND_CLASSIC_DATE);
  text_layer_set_text_color(s_layer_date, prv_classic_complication_color());
  text_layer_set_background_color(s_layer_date, GColorClear);
  text_layer_set_text_alignment(s_layer_date, GTextAlignmentCenter);
  text_layer_set_font(s_layer_date, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_layer_date));
  classic_update_date();

  if (s_settings.DoWeather) {
    //Weather layer
    s_layer_weather = text_layer_create(BOUND_CLASSIC_WEATHER);
    text_layer_set_text_color(s_layer_weather, prv_classic_complication_color());
    text_layer_set_background_color(s_layer_weather, GColorClear);
    text_layer_set_text_alignment(s_layer_weather, GTextAlignmentCenter);
    text_layer_set_font(s_layer_weather, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(s_layer_weather, "Weather!");
    layer_add_child(window_layer, text_layer_get_layer(s_layer_weather));
  }

  classic_ui_set_hidden(s_settings.HideUI);
}

void classic_window_unload(Window *window) {
  layer_destroy(s_layer_background);
  text_layer_destroy(s_layer_date);
  text_layer_destroy(s_layer_weather);

  if (classic_font_time) { fonts_unload_custom_font(classic_font_time); }

  if (s_pdc_classic_carets) { gdraw_command_image_destroy(s_pdc_classic_carets); }
  if (s_pdc_classic_jewel) { gdraw_command_image_destroy(s_pdc_classic_jewel); }
  if (s_pdc_classic_jewel_stroke) { gdraw_command_image_destroy(s_pdc_classic_jewel_stroke); }
}