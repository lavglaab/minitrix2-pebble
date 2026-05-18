#include <pebble.h>
#include "omni.h"
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

static GDrawCommandImage *s_pdc_omni_jewel;
static GDrawCommandImage *s_pdc_omni_carets;
static GDrawCommandImage *s_pdc_omni_caret_stroke;

static GColor omni_color_main; //These don't actually change in
static GColor omni_color_data; //flight, so we can keep them up here
static GColor omni_color_status; // this one does though

static bool omni_time_showing = false;

/* ---------- UI ----------*/
static GColor prv_omni_color() {
  GColor color = PAL_OMNI_STRIPES;
  #if defined(PBL_COLOR)
  if (settings_get()->DoColorOverride) { color = settings_get()->CustomColor; }
  #endif
  return color;
}

static GColor prv_omni_color_data() {
  GColor color = PAL_OMNI_DATA;
  #if defined(PBL_COLOR)
  if (settings_get()->HighContrast) {
    color = gcolor_legible_over(prv_omni_color());
    return color;
  }

  if (settings_get()->DoColorOverride) { color = gcolor_legible_over(settings_get()->CustomColor); }
  #endif
  return color;
}

static GColor prv_omni_color_status() {
  GColor color = PAL_OMNI_BG;
  if (battery_state_service_peek().charge_percent <= 10) { color = PAL_OMNI_STATUS_LOWBATT; }
  if (!connection_service_peek_pebble_app_connection()) { color = PAL_OMNI_STATUS_DISCONNECT; }
  return color;
}

void omni_update_minute() {
  layer_mark_dirty(s_layer_background);
}

void omni_update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char buffer_date[16];
  strftime(buffer_date, sizeof(buffer_date), "%a%n%b%e", tick_time);
  text_layer_set_text(s_layer_date, buffer_date);
}

void omni_update_weather(int response_code) {
  // set textlayer and icon
}

void omni_update_style() {
    // Set colors
    omni_color_status = prv_omni_color_status();
    omni_color_data = prv_omni_color_data();
    omni_color_main = prv_omni_color();

    // Recolor text
    text_layer_set_text_color(s_layer_date, omni_color_data);
    if (s_layer_weather) { text_layer_set_text_color(s_layer_weather, omni_color_data); }

    // Redraw everything
    layer_mark_dirty(s_layer_background);
}

void omni_ui_set_hidden(bool value) {
  omni_time_showing = !value;
  layer_set_hidden(text_layer_get_layer(s_layer_date), value);
  if (settings_get()->DoWeather) { layer_set_hidden(text_layer_get_layer(s_layer_weather), value); }
  layer_mark_dirty(s_layer_background);
}

static void prv_init_pdc_images() {
    if (!s_pdc_omni_jewel) { s_pdc_omni_jewel = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_OMNI_JEWEL); }
    if (!s_pdc_omni_carets) { s_pdc_omni_carets = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_OMNI_CARETS); }
    if (!s_pdc_omni_caret_stroke) { s_pdc_omni_caret_stroke = gdraw_command_image_create_with_resource(RESOURCE_ID_PATH_OMNI_CARET_STROKE); }
}

/* ---------- Update Procs ---------- */
static void prv_init_fonts() {
    #if defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_EMERY) // gross, ew, but i dont think theres a better way to pack an xl font
    if (s_font_time_large == NULL) {
        s_font_time_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ALATSI_70));
    }
    #endif
    if (s_font_time_medium == NULL) {
        s_font_time_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ALATSI_56));
    }
}

static GFont prv_select_font_for_size(Layer *layer) {
    int height = layer_get_bounds(layer).size.h;
    #if defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_EMERY) // gross, ew, but i dont think theres a better way to pack an xl font
    if (height > 180) {
        // unobstructedbounds is tall enough to use the larger font
        if (s_font_time_large == NULL) {
            prv_init_fonts();
        }
        return s_font_time_large;
    }
    #endif
    // use the regular basalt-sized font
    if (s_font_time_medium == NULL) {
        prv_init_fonts();
    }
    return s_font_time_medium;
}

static void update_proc_omni_bg(Layer *layer, GContext *ctx) {
    LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "update_proc_omni_bg");
    GRect bounds = layer_get_unobstructed_bounds(layer);

    // make sure pdcs are loaded
    prv_init_pdc_images();
    // now resize them to fit the current bounds
    draw_command_image_fill_size(s_pdc_omni_jewel, bounds.size);
    draw_command_image_fill_size(s_pdc_omni_carets, bounds.size);
    draw_command_image_fill_size(s_pdc_omni_caret_stroke, bounds.size);

    // center scaled pdcs within display bounds
    // figure out how much larger image is than screen, and offset by half that
    GSize full_size = gdraw_command_image_get_bounds_size(s_pdc_omni_carets);
    int delta_x = full_size.w - bounds.size.w;
    int delta_y = full_size.h - bounds.size.h;

    if (delta_x != 0) { delta_x = (delta_x / 2) * -1; }
    if (delta_y != 0) { delta_y = (delta_y / 2) * -1; }

    GPoint image_origin = GPoint(delta_x, delta_y);

    //Clear canvas
    graphics_context_set_fill_color(ctx, PAL_OMNI_BG);
    graphics_fill_rect(ctx, bounds, 0, GCornersAll);

    //Status jewel
    draw_command_image_recolor(s_pdc_omni_jewel, omni_color_status);
    gdraw_command_image_draw(ctx, s_pdc_omni_jewel, image_origin);

    //Green carets
    draw_command_image_recolor(s_pdc_omni_carets, omni_color_main);
    gdraw_command_image_draw(ctx, s_pdc_omni_carets, image_origin);

    // Draw time
    if (!settings_get()->HideUI || omni_time_showing) {
        GFont time_font = prv_select_font_for_size(layer);
        graphics_context_set_text_color(ctx, omni_color_data);
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

        bool strip_leading_zero = !clock_is_24h_style();
        if (strip_leading_zero) {
            if ((char)time[0] == '0') {
                time[0] = time[1];
                time[1] = 'X';
            }
        }

        const int digit_places = 4;
        const int digits_per_place = 1;

        for (uint8_t i = 0; i < digit_places; i++) {
            // split off the char for the digit i need
            static char buf[2];
            strncpy(buf, time + (i * digits_per_place), digits_per_place);

            if (strcmp(buf, "X") == 0) { continue; }

            GPoint point;
            GPoint origin = GPoint(50, 50);
            if (3 > i && i > 0) { // we have two points we use in the pattern A, B, B, A
                point = POINT_OMNI_TIME_SECOND_LEFT;
            } else {
                point = POINT_OMNI_TIME_TOP_LEFT;
            }

            // Translate coordinate plane
            point = GPoint(point.x - origin.x, point.y - origin.y);

            bool flip = (i >= 2); // minute digits go on the bottom right

            point = GPoint(
                point.x * (flip ? -1 : 1),  // invert, or do not invert, coordinate
                point.y * (flip ? -1 : 1)
            );

            // Translate coordinate plane back
            point = GPoint(point.x + origin.x, point.y + origin.y);

            // Scale from 100x100 coordinate to displaysplace coordinate
            point = scale_gpoint(point, (bounds.size.w / 100.0), (bounds.size.h / 100.0));
            text_draw_centered(ctx, buf, time_font, point, TEXT_OFFSET_ALATSI);
        }
    }

    //Caret stroke
    gdraw_command_image_draw(ctx, s_pdc_omni_caret_stroke, image_origin);
}

/* ---------- Life cycle ----------*/

void omni_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_layer_background = layer_create(bounds);
  layer_set_update_proc(s_layer_background, update_proc_omni_bg);
  layer_add_child(window_layer, s_layer_background);

  //Date layer
  s_layer_date = text_layer_create(BOUND_OMNI_DATE);
  text_layer_set_background_color(s_layer_date, GColorClear);
  text_layer_set_text_alignment(s_layer_date, GTextAlignmentLeft);
  text_layer_set_font(s_layer_date, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_layer_date));
  omni_update_date();

  if (settings_get()->DoWeather) {
    //Weather layer
    s_layer_weather = text_layer_create(BOUND_OMNI_WEATHER);
    text_layer_set_background_color(s_layer_weather, GColorClear);
    text_layer_set_text_alignment(s_layer_weather, GTextAlignmentRight);
    text_layer_set_font(s_layer_weather, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(s_layer_weather, "Weather!");
    layer_add_child(window_layer, text_layer_get_layer(s_layer_weather));
  }

  omni_update_style();
  omni_ui_set_hidden(settings_get()->HideUI);
}

void omni_window_unload(Window *window) {
  layer_destroy(s_layer_background);
  text_layer_destroy(s_layer_date);
  text_layer_destroy(s_layer_weather);

  if (s_font_time_medium) { fonts_unload_custom_font(s_font_time_medium); }
  if (s_font_time_large) { fonts_unload_custom_font(s_font_time_large); }

  gdraw_command_image_destroy(s_pdc_omni_jewel);
  gdraw_command_image_destroy(s_pdc_omni_carets);
  gdraw_command_image_destroy(s_pdc_omni_caret_stroke);
}
