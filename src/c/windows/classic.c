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

static GFont s_font_time_medium;
static GFont s_font_time_large;

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
  if (settings_get()->DoColorOverride) { color = settings_get()->CustomColor; }
  if (battery_state_service_peek().charge_percent <= 10) { color = PAL_CLASSIC_STATUS_LOWBATT; }
  if (!connection_service_peek_pebble_app_connection()) { color = PAL_CLASSIC_STATUS_DISCONNECT; }
  #endif
  return color;
}

static GColor prv_classic_complication_color() {
  GColor color = PAL_CLASSIC_COMPLICATIONS;
  #if defined(PBL_COLOR)

  if (settings_get()->HighContrast) {
    color = gcolor_legible_over(prv_classic_jewel_color());
    return color;
  }

  if (settings_get()->DoColorOverride ) { color = gcolor_legible_over(settings_get()->CustomColor); }
  if (battery_state_service_peek().charge_percent <= 10) { color = PAL_CLASSIC_TEXT_LOWBATT; }
  if (!connection_service_peek_pebble_app_connection()) { color = PAL_CLASSIC_TEXT_DISCONNECT; }
  #endif
  return color;
}

static GColor prv_classic_background_color() {
    return (settings_get()->DialMode == 'c') ? PAL_CLASSIC_GREY_CARETS : PAL_CLASSIC_BLACK_CARETS;
}

void classic_update_minute() {
  layer_mark_dirty(s_layer_background);
}

void classic_update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char buffer_date[16];
  strftime(buffer_date, sizeof(buffer_date), PBL_IF_RECT_ELSE("%a %b %e", "%a%n%b %e"), tick_time);
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

    // obelix only - color backliught
    #if PBL_API_EXISTS(light_set_color)
        if (settings_get()->DoColorBacklight) {
            light_set_color(classic_color_jewel);
        } else {
            light_set_system_color();
        }
    #endif
}

void classic_ui_set_hidden(bool value) {
  classic_time_showing = !value;
  layer_set_hidden(text_layer_get_layer(s_layer_date), value);
  if (settings_get()->DoWeather) { layer_set_hidden(text_layer_get_layer(s_layer_weather), value); }
  layer_mark_dirty(s_layer_background);
}

static void prv_init_pdc_images() {
    if (!s_pdc_classic_carets) { s_pdc_classic_carets = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_CARETS); }
    if (!s_pdc_classic_jewel) { s_pdc_classic_jewel = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_JEWEL); }
    if (!s_pdc_classic_jewel_stroke) { s_pdc_classic_jewel_stroke = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_CLASSIC_JEWEL_STROKE); }
}

/* ---------- Update procs ---------- */
static void prv_init_fonts() {
    if (s_font_time_large == NULL) {
        s_font_time_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BALOO_77));
    }
    if (s_font_time_medium == NULL) {
        s_font_time_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BALOO_60));
    }
}

static GFont prv_select_font_for_size(Layer *layer) {
    int height = layer_get_bounds(layer).size.h;
    if (height > 180) {
        // unobstructedbounds is tall enough to use the larger font
        if (s_font_time_large == NULL) {
            prv_init_fonts();
        }
        return s_font_time_large;
    } else {
        // use the regular basalt-sized font
        if (s_font_time_medium == NULL) {
            prv_init_fonts();
        }
        return s_font_time_medium;
    }
}

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

    GPoint image_origin = GPoint(delta_x, delta_y);

    //Clear canvas
    graphics_context_set_fill_color(ctx, classic_color_background);
    graphics_fill_rect(ctx, bounds, 0, GCornersAll);


    //Black carets
    gdraw_command_image_draw(ctx, s_pdc_classic_carets, image_origin);

    // Draw time
    if (!settings_get()->HideUI || classic_time_showing) {
        GFont time_font = prv_select_font_for_size(layer);
        graphics_context_set_text_color(ctx, PAL_CLASSIC_CLOCK);
        // Get a tm structure
        time_t temp = time(NULL);
        struct tm *tick_time = localtime(&temp);

        // Write the current hours and minutes into a buffer
        char time[5];
        #if defined (DEBUG_UI_DUMMYMODE)
        strncpy(time, "1234", 5); // debug time
        #else
        strftime(time, sizeof(time), clock_is_24h_style() ? "%H%M" : "%I%M", tick_time); // real time
        #endif

        #if !defined(PBL_ROUND)
        bool strip_leading_zero = !clock_is_24h_style();
        if (strip_leading_zero) {
            if ((char)time[0] == '0') {
                time[0] = time[1];
                time[1] = 'X';
            }
        }
        #endif

        // Values to adjust layout between round and rect
        const int digit_places = PBL_IF_ROUND_ELSE(2, 4);
        const int digits_per_place = PBL_IF_ROUND_ELSE(2, 1);

        for (uint8_t i = 0; i < digit_places; i++) {
            // split off the char for the digit i need
            static char buf[3];
            strncpy(buf, time + (i * digits_per_place), digits_per_place);

            if (strcmp(buf, "X") == 0) { continue; }

            GPoint point = PBL_IF_ROUND_ELSE(POINT_CLASSIC_TIME_LEFT, POINT_CLASSIC_TIME_TOP_LEFT);
            GPoint origin = GPoint(50, 50);

            // Translate coordinate plane
            point = GPoint(point.x - origin.x, point.y - origin.y);

            bool inv_x = i & 0b00000001; //i can get this value from the rightmost bit of int i
            bool inv_y = i & 0b00000010; // i can get this value from the second-to-rightmost bit of int i

            #if defined(PBL_RECT)
            if (settings_get()->RectClassicClockLtR) {
                // swap which axes get inverted, thereby drawing left->right instead of top->bottom
                bool temp = inv_x;
                inv_x = inv_y;
                inv_y = temp; // fun little shell game
            }
            #endif

            point = GPoint(
                point.x * (inv_x ? -1 : 1),  // invert, or do not invert, coordinate
                point.y * (inv_y ? -1 : 1)
            );

            // Translate coordinate plane back
            point = GPoint(point.x + origin.x, point.y + origin.y);

            // Scale from 100x100 coordinate to displaysplace coordinate
            point = scale_gpoint(point, (bounds.size.w / 100.0), (bounds.size.h / 100.0));
            text_draw_centered(ctx, buf, time_font, point, TEXT_OFFSET_BALOO);
        }
    }

    //Jewel
    draw_command_image_recolor(s_pdc_classic_jewel, classic_color_jewel);
    gdraw_command_image_draw(ctx, s_pdc_classic_jewel, image_origin);

    //Jewel stroke
    draw_command_image_recolor(s_pdc_classic_jewel_stroke, GColorBlack);
    gdraw_command_image_draw(ctx, s_pdc_classic_jewel_stroke, image_origin);
}

/* ---------- Life cycle ----------*/

static void prv_date_layer_create(Window *window) {
    GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(window));
    GSize size_date = text_measure_simply("two lines\nof type", text_get_complication_font(), GRect(0, 0, bounds.size.w, 100));

    GRect bounds_date = GRect(
        0, PBL_IF_ROUND_ELSE(8, 0), bounds.size.w, size_date.h * 3
    );
    s_layer_date = text_layer_create(bounds_date);
    text_layer_set_text_color(s_layer_date, prv_classic_complication_color());
    text_layer_set_background_color(s_layer_date, GColorClear);
    text_layer_set_text_alignment(s_layer_date, GTextAlignmentCenter);
    text_layer_set_font(s_layer_date, text_get_complication_font());
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_layer_date));
    classic_update_date();
}

void classic_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //Render layer with time
  s_layer_background = layer_create(bounds);
  layer_set_update_proc(s_layer_background, update_proc_classic_bg);
  layer_add_child(window_layer, s_layer_background);

  //Date layer
  prv_date_layer_create(window);

  if (settings_get()->DoWeather) {
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
  classic_ui_set_hidden(settings_get()->HideUI);
}

void classic_window_unload(Window *window) {
  layer_destroy(s_layer_background);
  text_layer_destroy(s_layer_date);
  text_layer_destroy(s_layer_weather);

  if (s_font_time_medium) { fonts_unload_custom_font(s_font_time_medium); }
  if (s_font_time_large) { fonts_unload_custom_font(s_font_time_large); }


  gdraw_command_image_destroy(s_pdc_classic_carets);
  gdraw_command_image_destroy(s_pdc_classic_jewel);
  gdraw_command_image_destroy(s_pdc_classic_jewel_stroke);
}
