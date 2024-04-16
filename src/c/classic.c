#include <pebble.h>
#include "classic.h"
#include "settings.h"
#include "classic_paths.c"
#include "debug_flags.h"

static Layer *s_layer_background;
static TextLayer *s_layer_date;
static TextLayer *s_layer_weather;

static GFont classic_font_time;

static GPath *s_path_carets_black;
static GPath *s_path_jewel;
static GPath *s_path_jewelstroke;

static GColor classic_color_background;

static bool classic_time_showing;
static AppTimer *s_tap_timer;

ClaySettings classic_settings;

/* ----------- Settings funcs ----------*/

static void classic_default_settings() {
  classic_settings.DialMode = 'c';
  classic_settings.HideUI = false;
  classic_settings.DoColorOverride = false;
  classic_settings.CustomColor = GColorWhite;
  classic_settings.HighContrast = false;
  classic_settings.DoWeather = false;
}

static void classic_load_settings() {
  classic_default_settings();
  persist_read_data(SETTINGS_KEY, &classic_settings, sizeof(classic_settings));
}

/* ---------- Event Service Handlers ---------- */

static void prv_classic_update_style(); //Defined in UI section
static void prv_classic_update_date(); //Defined in UI section

static void classic_tick_handler(struct tm *tick_time, TimeUnits changed) {
  layer_mark_dirty(s_layer_background);
  if ((changed & DAY_UNIT) != 0) {
    prv_classic_update_date();
    }
  if ((changed & HOUR_UNIT) != 0 && classic_settings.DoWeather) { /*request weather*/ }
}

static void classic_battery_callback(BatteryChargeState state) {
  prv_classic_update_style();
}

static void classic_bluetooth_callback(bool connected) {
  prv_classic_update_style();
  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void prv_classic_ui_set_hidden(bool value); //Defined in UI section
static void classic_tap_timer_handler(void *data) {
  if (classic_settings.HideUI) {
    prv_classic_ui_set_hidden(true);
    s_tap_timer = NULL;
  }
}

static void classic_accel_tap_handler(AccelAxisType axis, int32_t direction) {
  prv_classic_ui_set_hidden(false);

  //Schedule timer to re-hide UI
  if (s_tap_timer && app_timer_reschedule(s_tap_timer, 5000)) {
    return;
  }
  s_tap_timer = app_timer_register(5000, classic_tap_timer_handler, 0);
}

/* ---------- UI ----------*/



static void prv_classic_update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char buffer_date[16];
  strftime(buffer_date, sizeof(buffer_date), PBL_IF_RECT_ELSE("%a %b%e", "%a%n%b%e"), tick_time);
  text_layer_set_text(s_layer_date, buffer_date);
}

static GColor prv_classic_jewel_color() {
  GColor color = PAL_CLASSIC_JEWEL;
  #if defined(PBL_COLOR)
  if (classic_settings.DoColorOverride) { color = classic_settings.CustomColor; }
  if (battery_state_service_peek().charge_percent <= 10) { color = PAL_CLASSIC_STATUS_LOWBATT; }
  if (!connection_service_peek_pebble_app_connection()) { color = PAL_CLASSIC_STATUS_DISCONNECT; }
  #endif
  return color;
}

static GColor prv_classic_complication_color() {
  GColor color = PAL_CLASSIC_COMPLICATIONS;
  #if defined(PBL_COLOR)

  if (classic_settings.HighContrast) {
    color = gcolor_legible_over(prv_classic_jewel_color());
    return color;
  }

  if (classic_settings.DoColorOverride ) { color = gcolor_legible_over(classic_settings.CustomColor); }
  if (battery_state_service_peek().charge_percent <= 10) { color = PAL_CLASSIC_TEXT_LOWBATT; }
  if (!connection_service_peek_pebble_app_connection()) { color = PAL_CLASSIC_TEXT_DISCONNECT; }
  #endif
  return color;
}

static void prv_classic_update_style() {
  layer_mark_dirty(s_layer_background);
  text_layer_set_text_color(s_layer_date, prv_classic_complication_color());
  if (s_layer_weather) { text_layer_set_text_color(s_layer_weather, prv_classic_complication_color()); }
}

static void prv_classic_ui_set_hidden(bool value) {
  if (value) {
    tick_timer_service_unsubscribe();
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT | HOUR_UNIT | DAY_UNIT, classic_tick_handler);
  }
  
  classic_time_showing = !value;
  layer_set_hidden(text_layer_get_layer(s_layer_date), value);
  if (classic_settings.DoWeather) { layer_set_hidden(text_layer_get_layer(s_layer_weather), value); }
  layer_mark_dirty(s_layer_background);
}

static void update_proc_classic_bg(Layer *layer, GContext *ctx) {
    LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "update_proc_classic_bg");
    GRect bounds = layer_get_bounds(layer);

    if (!s_path_carets_black) { s_path_carets_black = gpath_create(&PATH_BLACK_CARET); }
    if (!s_path_jewel) {s_path_jewel = gpath_create(&PATH_JEWEL); }
    if (!s_path_jewelstroke) { s_path_jewelstroke = gpath_create(&PATH_JEWELSTROKE); }
    
    //Clear canvas
    graphics_context_set_fill_color(ctx, classic_color_background);
    graphics_fill_rect(ctx, bounds, 0, GCornersAll);
    //Black carets
    graphics_context_set_fill_color(ctx, PAL_CLASSIC_BLACK_CARETS);
    gpath_rotate_to(s_path_carets_black, DEG_TO_TRIGANGLE(0));
    gpath_move_to(s_path_carets_black, GPoint(-1, 0));
    gpath_draw_filled(ctx, s_path_carets_black);
    gpath_rotate_to(s_path_carets_black, DEG_TO_TRIGANGLE(180));
    gpath_move_to(s_path_carets_black, GPoint(bounds.size.w, bounds.size.h));
    gpath_draw_filled(ctx, s_path_carets_black);

    // Draw time
    if (!classic_settings.HideUI || classic_time_showing) {
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
    graphics_context_set_fill_color(ctx, prv_classic_jewel_color());
    gpath_move_to(s_path_jewel, GPoint(0, 0));
    gpath_draw_filled(ctx, s_path_jewel);
    
    //Jewel stroke
    graphics_context_set_stroke_color(ctx, PAL_CLASSIC_BLACK_CARETS);
    graphics_context_set_stroke_width(ctx, 2);
    gpath_rotate_to(s_path_jewelstroke, DEG_TO_TRIGANGLE(0));
    gpath_move_to(s_path_jewelstroke, GPoint(-1, 0));
    gpath_draw_outline_open(ctx, s_path_jewelstroke);

    gpath_rotate_to(s_path_jewelstroke, DEG_TO_TRIGANGLE(180));
    gpath_move_to(s_path_jewelstroke, GPoint(bounds.size.w, bounds.size.h-1));
    gpath_draw_outline_open(ctx, s_path_jewelstroke);
}

/* ---------- Life cycle ----------*/

static void classic_window_load(Window *window) {
  classic_load_settings();

  //Set up event handlers
  tick_timer_service_subscribe(MINUTE_UNIT | HOUR_UNIT | DAY_UNIT, classic_tick_handler);
  battery_state_service_subscribe(classic_battery_callback);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = classic_bluetooth_callback
  });
  if (classic_settings.HideUI) { accel_tap_service_subscribe(classic_accel_tap_handler); }

  //Set background color according to classic or uaf
  classic_color_background = (classic_settings.DialMode == 'c') 
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
  prv_classic_update_date();

  if (classic_settings.DoWeather) {
    //Weather layer
    s_layer_weather = text_layer_create(BOUND_CLASSIC_WEATHER);
    text_layer_set_text_color(s_layer_weather, prv_classic_complication_color());
    text_layer_set_background_color(s_layer_weather, GColorClear);
    text_layer_set_text_alignment(s_layer_weather, GTextAlignmentCenter);
    text_layer_set_font(s_layer_weather, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(s_layer_weather, "Weather!");
    layer_add_child(window_layer, text_layer_get_layer(s_layer_weather));
  }

  prv_classic_ui_set_hidden(classic_settings.HideUI);
}

static void classic_window_unload(Window *window) {
  layer_destroy(s_layer_background);
  text_layer_destroy(s_layer_date);
  text_layer_destroy(s_layer_weather);

  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  accel_tap_service_unsubscribe();
}