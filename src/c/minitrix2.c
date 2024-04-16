#include <pebble.h>
#include "settings.h"
#include "classic.c"
#include "omni.c"
#include "debug_flags.h"

static Window *s_window_classic;
static Window *s_window_omni;

ClaySettings settings;

/* ----------- Settings funcs ----------*/

static void prv_default_settings() {
  settings.DialMode = 'c';
  settings.HideUI = false;
  settings.DoColorOverride = false;
  settings.CustomColor = GColorWhite;
  settings.HighContrast = false;
  settings.DoWeather = false;
}

static void prv_load_settings() {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

/* ---------- Message handling ---------- */

static void push_proper_dial_window();

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // UI
  Tuple *dial_mode_t = dict_find(iter, MESSAGE_KEY_PrefDialMode);
  if(dial_mode_t) {
    settings.DialMode = dial_mode_t->value->int8;
  }

  Tuple *hide_ui_t = dict_find(iter, MESSAGE_KEY_PrefHideUI);
  if(hide_ui_t) { settings.HideUI = hide_ui_t->value->int32 == 1; }

  Tuple *high_contrast_t = dict_find(iter, MESSAGE_KEY_PrefHighContrast);
  if(high_contrast_t) { settings.HighContrast = high_contrast_t->value->int32 == 1; }

  Tuple *do_color_override_t = dict_find(iter, MESSAGE_KEY_PrefDoColorOverride);
  if(do_color_override_t) { settings.DoColorOverride = do_color_override_t->value->int32 == 1; }

  Tuple *custom_color_t = dict_find(iter, MESSAGE_KEY_PrefOverrideColor);
  if(custom_color_t) { settings.CustomColor = GColorFromHEX(custom_color_t->value->int32); }

  //Features
  Tuple *do_weather_t = dict_find(iter, MESSAGE_KEY_PrefDoWeather);
  if(do_weather_t) { settings.DoWeather = do_weather_t->value->int32 == 1; }

  prv_save_settings();

  if(dial_mode_t || hide_ui_t || do_color_override_t || custom_color_t || do_weather_t) {
    push_proper_dial_window();
  }
}

/* ---------- Life cycle ---------- */

const bool animated = true;

static void prv_push_window_classic() {
  if (s_window_classic) {
    window_destroy(s_window_classic);
    s_window_classic = NULL;  
  }
  s_window_classic = window_create();
  window_set_window_handlers(s_window_classic, (WindowHandlers) {
    .load = classic_window_load,
    .unload = classic_window_unload,
  });
  window_stack_push(s_window_classic, animated);
}

static void prv_push_window_omni() {
  if (s_window_omni) {
    window_destroy(s_window_omni);
    s_window_omni = NULL;  
  }
  s_window_omni = window_create();
  window_set_window_handlers(s_window_omni, (WindowHandlers) {
    .load = omni_window_load,
    .unload = omni_window_unload,
  });
  window_stack_push(s_window_omni, animated);
}

static void push_proper_dial_window() {
  window_stack_pop_all(animated);
  if (settings.DialMode == 'o') {
    prv_push_window_omni();
  } else {
    prv_push_window_classic();
  }
}

static void prv_init(void) {
  prv_load_settings();
  
  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);
  
  push_proper_dial_window();
}

static void prv_deinit(void) {
  window_destroy(s_window_classic);
  window_destroy(s_window_omni);
}

int main(void) {
  prv_init();

  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window");

  app_event_loop();
  prv_deinit();
}
