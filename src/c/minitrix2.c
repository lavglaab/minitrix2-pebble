#include <pebble.h>
#include "settings.h"
#include "weather.h"
#include "classic.c"
#include "omni.c"

#include "debug_flags.h"

static bool s_js_ready = false;

static Window *s_window_classic;
static Window *s_window_omni;

ClaySettings s_settings;
WeatherData s_weather;

/* ----------- Settings funcs ----------*/

static void prv_default_settings() {
  s_settings.DialMode = 'c';
  s_settings.HideUI = false;
  s_settings.DoColorOverride = false;
  s_settings.CustomColor = GColorWhite;
  s_settings.HighContrast = false;
  s_settings.DoWeather = false;
  s_settings.WeatherUnits = 'f';
}

static void prv_load_settings() {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

/* ---------- Weather ---------- */

static void prv_load_weather() {
  s_weather.Condition = 0;
  s_weather.TemperatureKelvin = 0;
  s_weather.Timestamp = 0;
  persist_read_data(WEATHER_KEY, &s_weather, sizeof(s_weather));
}

static void prv_save_weather() {
  persist_write_data(WEATHER_KEY, &s_weather, sizeof(s_weather));
}

static bool request_new_weather() {
  if (!s_js_ready) { return false; }

  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if(result != APP_MSG_OK) {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
    return false;
  }

  // Add an item to ask for weather data
  int value = 1;
  dict_write_int(out_iter, MESSAGE_KEY_WeatherRequestPls, &value, sizeof(int), true);

  // Send this message
  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    return false;
  }
  return true;
}

static bool check_weather() {
  prv_load_weather();
  time_t now_time = time(NULL);
  if (now_time > s_weather.Timestamp) {
    // Stored weather is expired or nonexistent
    return request_new_weather();
  }

  // Continue with saved weather
  // display_update_weather(*s_weather);
  return true;
}

/* ---------- Message handling ---------- */

static void push_proper_dial_window();

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  /* ---- Comms state ---- */
  Tuple *js_ready_t = dict_find(iter, MESSAGE_KEY_JSReady);
  if(js_ready_t && s_settings.DoWeather) {
    s_js_ready = true;
    LOG_IF_ENABLED(DEBUG_LOG_WEATHER, APP_LOG_LEVEL_INFO, "JS is ready! Asking for weather");
    request_new_weather();
  }
  
  /* ---- Settings ---- */
  // UI
  Tuple *dial_mode_t = dict_find(iter, MESSAGE_KEY_PrefDialMode);
  if(dial_mode_t) {
    s_settings.DialMode = dial_mode_t->value->int8;
  }

  Tuple *hide_ui_t = dict_find(iter, MESSAGE_KEY_PrefHideUI);
  if(hide_ui_t) { s_settings.HideUI = hide_ui_t->value->int32 == 1; }

  Tuple *high_contrast_t = dict_find(iter, MESSAGE_KEY_PrefHighContrast);
  if(high_contrast_t) { s_settings.HighContrast = high_contrast_t->value->int32 == 1; }

  Tuple *do_color_override_t = dict_find(iter, MESSAGE_KEY_PrefDoColorOverride);
  if(do_color_override_t) { s_settings.DoColorOverride = do_color_override_t->value->int32 == 1; }

  Tuple *custom_color_t = dict_find(iter, MESSAGE_KEY_PrefOverrideColor);
  if(custom_color_t) { s_settings.CustomColor = GColorFromHEX(custom_color_t->value->int32); }

  //Features
  Tuple *do_weather_t = dict_find(iter, MESSAGE_KEY_PrefDoWeather);
  if(do_weather_t) { s_settings.DoWeather = do_weather_t->value->int32 == 1; }

  Tuple *weather_units_t = dict_find(iter, MESSAGE_KEY_PrefWeatherUnits);
  if(weather_units_t) {
    s_settings.WeatherUnits = weather_units_t->value->int8;
  }

  prv_save_settings();

  if(dial_mode_t || hide_ui_t || do_color_override_t || custom_color_t || do_weather_t) {
    push_proper_dial_window();
  }

  /* ---- Weather data ---- */
  Tuple *weather_returncode_t = dict_find(iter, MESSAGE_KEY_WeatherReturnCode);
  if(weather_returncode_t) {
    // We either got new weather, or an error
    int weather_returncode = weather_returncode_t->value->int32;

    switch (weather_returncode) {
      case 0:
        // OK, we should also have data
        {
        Tuple *weather_temp_t = dict_find(iter, MESSAGE_KEY_WeatherTemperatureK);
        Tuple *weather_condition_t = dict_find(iter, MESSAGE_KEY_WeatherCondition);
        time_t now_time = time(NULL);
        s_weather.TemperatureKelvin = weather_temp_t->value->int32;
        s_weather.Condition = weather_condition_t->value->int32;
        s_weather.Timestamp = now_time + (60 * 60);
        prv_save_weather();

        // Ask watchface to update the display
        }
        break;
      case 1:
        // Some kind of error fetching data

        break;
      case 2:
        // User hasn't provided a token

        break;
    }
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
  if (s_settings.DialMode == 'o') {
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
