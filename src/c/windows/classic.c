#include <pebble.h>
#include "classic.h"
#include "../settings.h"
#include "../weather.h"
#include "../debug_flags.h"
#include "../text_drawing.h"
#include "../pdc/colorize_pdc.h"
#include "../pdc/scale_pdc.h"

static Layer *s_layer_background;
static TextLayer *s_layer_date;
static TextLayer *s_layer_weather;

static GFont classic_font_time;

static GPoint s_image_origin = GPoint(0,0);

static GDrawCommandImage *s_pdc_classic_carets;
static GDrawCommandImage *s_pdc_classic_jewel;
static GDrawCommandImage *s_pdc_classic_jewel_stroke;

static GColor classic_color_background;
static GColor classic_color_jewel;

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

static GColor prv_classic_background_color() {
    return (s_settings.DialMode == 'c') ? PAL_CLASSIC_GREY_CARETS : PAL_CLASSIC_BLACK_CARETS;
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
    //Set colors
    classic_color_background = prv_classic_background_color();
    classic_color_jewel = prv_classic_jewel_color();

    // Recolor text
    text_layer_set_text_color(s_layer_date, prv_classic_complication_color());
    if (s_layer_weather) { text_layer_set_text_color(s_layer_weather, prv_classic_complication_color()); }

    // Redraw everything
    layer_mark_dirty(s_layer_background);
}

void classic_ui_set_hidden(bool value) {
  classic_time_showing = !value;
  layer_set_hidden(text_layer_get_layer(s_layer_date), value);
  if (s_settings.DoWeather) { layer_set_hidden(text_layer_get_layer(s_layer_weather), value); }
  layer_mark_dirty(s_layer_background);
}

static void prv_init_pdc_images() {
    if (!s_pdc_classic_carets) { s_pdc_classic_carets = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_CARETS); }
    if (!s_pdc_classic_jewel) { s_pdc_classic_jewel = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_JEWEL); }
    if (!s_pdc_classic_jewel_stroke) { s_pdc_classic_jewel_stroke = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_JEWEL_STROKE); }
}

/* ---------- Update procs ---------- */
static void update_proc_classic_bg(Layer *layer, GContext *ctx) {
    LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "update_proc_classic_bg");
    GRect bounds = layer_get_unobstructed_bounds(layer);

    // make sure pdcs are loaded
    prv_init_pdc_images();
    // now resize them to fit the current bounds
    draw_command_image_fill_size(s_pdc_classic_carets, bounds.size);
    draw_command_image_fill_size(s_pdc_classic_jewel, bounds.size);
    draw_command_image_fill_size(s_pdc_classic_jewel_stroke, bounds.size);

    // center scaled pdcs within display bounds
    // figure out how much larger image is than screen, and offset by half that
    GSize full_size = gdraw_command_image_get_bounds_size(s_pdc_classic_jewel);
    int delta_x = full_size.w - bounds.size.w;
    int delta_y = full_size.h - bounds.size.h;

    if (delta_x != 0) { delta_x = (delta_x / 2) * -1; }
    if (delta_y != 0) { delta_y = (delta_y / 2) * -1; }

    s_image_origin = GPoint(delta_x, delta_y);

    //Clear canvas
    graphics_context_set_fill_color(ctx, classic_color_background);
    graphics_fill_rect(ctx, bounds, 0, GCornersAll);


    //Black carets
    gdraw_command_image_draw(ctx, s_pdc_classic_carets, s_image_origin);

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
    char time[5];
    strftime(time, sizeof(time), clock_is_24h_style() ?
                                        "%H%M" : "%I%M", tick_time);

    //TODO: so we need to figure out a nice clean loopable way to
    // - make that gpoint make sense in a non-square rectangle (uneven scale factors?)
    // oh, and make that work with the HH-MM setup on round displays, too. can't forget that
    // this feels fizzbuzzy. like there should definitely be a mathematical solution to this.
    // getting a real boss baby vibe from this math problem.

    for (uint8_t i = 0; i < 4; i++) {
        // split off the char for the digit i need
        static char buf[2];
        strncpy(buf, time+i, 1);

        GPoint point = POINT_CLASSIC_TIME_TOP_LEFT;
        GPoint origin = GPoint(50, 50);

        // Translate coordinate plane
        point = GPoint(point.x - origin.x, point.y - origin.y);

        bool inv_x = i & 0b00000001; //i can get this value from the rightmost bit of int i
        bool inv_y = i & 0b00000010; // i can get this value from the second-to-rightmost bit of int i

        point = GPoint(
            point.x * (inv_x ? -1 : 1),  // invert, or do not invert, coordinate
            point.y * (inv_y ? -1 : 1)
        );

        // Translate coordinate plane back
        point = GPoint(point.x + origin.x, point.y + origin.y);

        // Scale from 100x100 coordinate to displaysplace coordinate
        point = scale_gpoint(point, (bounds.size.w / 100.0), (bounds.size.h / 100.0));
        text_draw_centered(ctx, buf, classic_font_time, point);
    }

    // #endif

    // #if defined(PBL_ROUND)
    // graphics_draw_text(ctx, classic_time_hour, classic_font_time, BOUND_CLASSIC_TIME_HOUR, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0); //Hour
    // graphics_draw_text(ctx, classic_time_minute, classic_font_time, BOUND_CLASSIC_TIME_MINUTE, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0); //Minute
    // #endif
    }

    //Jewel
    draw_command_image_recolor(s_pdc_classic_jewel, classic_color_jewel);
    gdraw_command_image_draw(ctx, s_pdc_classic_jewel, s_image_origin);

    //Jewel stroke
    gdraw_command_image_draw(ctx, s_pdc_classic_jewel_stroke, s_image_origin);
}

/* ---------- Life cycle ----------*/

void classic_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //Render layer with time
  s_layer_background = layer_create(bounds);
  layer_set_update_proc(s_layer_background, update_proc_classic_bg);
  layer_add_child(window_layer, s_layer_background);

  //Date layer
  s_layer_date = text_layer_create(BOUND_CLASSIC_DATE);
  // text_layer_set_text_color(s_layer_date, prv_classic_complication_color());
  text_layer_set_background_color(s_layer_date, GColorClear);
  text_layer_set_text_alignment(s_layer_date, GTextAlignmentCenter);
  text_layer_set_font(s_layer_date, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_layer_date));
  classic_update_date();

  if (s_settings.DoWeather) {
    //Weather layer
    s_layer_weather = text_layer_create(BOUND_CLASSIC_WEATHER);
    // text_layer_set_text_color(s_layer_weather, prv_classic_complication_color());
    text_layer_set_background_color(s_layer_weather, GColorClear);
    text_layer_set_text_alignment(s_layer_weather, GTextAlignmentCenter);
    text_layer_set_font(s_layer_weather, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(s_layer_weather, "Weather!");
    layer_add_child(window_layer, text_layer_get_layer(s_layer_weather));
  }

  classic_update_style();
  classic_ui_set_hidden(s_settings.HideUI);
}

void classic_window_unload(Window *window) {
  layer_destroy(s_layer_background);
  text_layer_destroy(s_layer_date);
  text_layer_destroy(s_layer_weather);

  if (classic_font_time) { fonts_unload_custom_font(classic_font_time); }


  gdraw_command_image_destroy(s_pdc_classic_carets);
  gdraw_command_image_destroy(s_pdc_classic_jewel);
  gdraw_command_image_destroy(s_pdc_classic_jewel_stroke);
}
