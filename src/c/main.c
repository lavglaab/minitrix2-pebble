#include <pebble.h>
#include "utils.h"
#include "classic.h"
#include "omni.h"

#include "debug_flags.h"

// Convenience macro for doing stuff based on dial mode
#define UI_IF_OMNI_ELSE(a, b) (s_settings.DialMode == 'o') ? (a) : (b)

static Window *s_window_classic;
static Window *s_window_omni;

static AppTimer *s_tap_timer;

/* ---------- Message handling ---------- */

static void push_proper_dial_window(); // Defined under Lifecycle

static void common_update_weather(int response_code); // defined under UI Common
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  /* ---- Comms state ---- */
  Tuple *js_ready_t = dict_find(iter, MESSAGE_KEY_JSReady);
  if(js_ready_t && s_settings.DoWeather) {
    s_js_ready = true;
    LOG_IF_ENABLED(DEBUG_LOG_WEATHER, APP_LOG_LEVEL_INFO, "JS is ready! Asking for weather");
    if (has_saved_weather()) {
      common_update_weather(0);
    } else {
      request_new_weather();
    }
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

  save_settings();

  if(dial_mode_t || hide_ui_t || do_color_override_t || custom_color_t || do_weather_t) {
    push_proper_dial_window();
  }

  /* ---- Weather data ---- */
  Tuple *weather_returncode_t = dict_find(iter, MESSAGE_KEY_WeatherReturnCode);
  if(weather_returncode_t) {
    // We either got new weather, or an error
    int weather_returncode = weather_returncode_t->value->int32;
    // LOG_IF_ENABLED(DEBUG_LOG_WEATHER, APP_LOG_LEVEL_DEBUG, "Weather returncode present");

    switch (weather_returncode) {
      case 0:
        // OK, we should also have data
        {
        LOG_IF_ENABLED(DEBUG_LOG_WEATHER, APP_LOG_LEVEL_DEBUG, "Weather return code 0, got data");
        Tuple *weather_temp_t = dict_find(iter, MESSAGE_KEY_WeatherTemperatureK);
        Tuple *weather_condition_t = dict_find(iter, MESSAGE_KEY_WeatherCondition);
        time_t now_time = time(NULL);
        s_weather.TemperatureKelvin = weather_temp_t->value->int32;
        s_weather.Condition = weather_condition_t->value->int32;
        s_weather.Timestamp = now_time + (60 * (1 * 60)); // Set timestamp for one hour from now
        LOG_IF_ENABLED(DEBUG_LOG_WEATHER, APP_LOG_LEVEL_DEBUG, "Received weather: Condition %d, Temp %d, Timestamp %d", (int)s_weather.Condition, (int)s_weather.TemperatureKelvin, (int)s_weather.Timestamp);
        save_weather();
        }
        break;
      case 1:
        // Some kind of error fetching data
        LOG_IF_ENABLED(DEBUG_LOG_WEATHER, APP_LOG_LEVEL_DEBUG, "Weather return code 1, error fetching data");
        break;
      case 2:
        // User hasn't provided a token
        LOG_IF_ENABLED(DEBUG_LOG_WEATHER, APP_LOG_LEVEL_DEBUG, "Weather return code 2, no token from user");
        break;
    }
    common_update_weather(weather_returncode); // Update dial display
  }
}

/* ---------- UI Common ---------- */
static void common_update_minute() {
  UI_IF_OMNI_ELSE(omni_update_minute(), classic_update_minute());
}

static void common_update_date() {
  UI_IF_OMNI_ELSE(omni_update_date(), classic_update_date());
}

static void common_update_style() {
  UI_IF_OMNI_ELSE(omni_update_style(), classic_update_style());
}

static void common_update_weather(int response_code) {
  UI_IF_OMNI_ELSE(omni_update_weather(response_code), classic_update_weather(response_code));
}

static void tick_handler(struct tm *tick_time, TimeUnits changed); // defined under Event Service Handlers
static void common_ui_set_hidden(bool value) {
  if (value) {
    tick_timer_service_unsubscribe();
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT | HOUR_UNIT | DAY_UNIT, tick_handler);
  }

  UI_IF_OMNI_ELSE(omni_ui_set_hidden(value), classic_ui_set_hidden(value));
}

/* ---------- Event Service Handlers ---------- */
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Tick handler fired");
  if ((changed & MINUTE_UNIT) != 0) { common_update_minute(); }
  if ((changed & DAY_UNIT) != 0) { common_update_date(); }
  if ((changed & HOUR_UNIT) != 0 && s_settings.DoWeather && !has_saved_weather()) { request_new_weather(); }
}

static void battery_callback(BatteryChargeState state) { common_update_style(); }

static void bluetooth_callback(bool connected) {
  common_update_style();
  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void tap_timer_handler(void *data) {
  if (s_settings.HideUI) {
    common_ui_set_hidden(true);
    s_tap_timer = NULL;
  }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  common_ui_set_hidden(false);

  //Schedule timer to re-hide UI
  if (s_tap_timer && app_timer_reschedule(s_tap_timer, 5000)) {
    return;
  } else {
    s_tap_timer = app_timer_register(5000, tap_timer_handler, 0);
  }
}

/* ---------- Life cycle ---------- */

const bool animated = true;

static void prv_push_window_classic() {
  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Pushing classic window");
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
  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Pushing omni window");
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

static void prv_window_load_common() {
  load_settings();

  // Clear any existing event handlers
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  accel_tap_service_unsubscribe();

  //Set up event handlers
  tick_timer_service_subscribe(MINUTE_UNIT | HOUR_UNIT | DAY_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  if (s_settings.HideUI) { accel_tap_service_subscribe(accel_tap_handler); }
}

static void push_proper_dial_window() {
  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Pushing proper dial window");
  window_stack_pop_all(animated);
  prv_window_load_common();
  UI_IF_OMNI_ELSE(prv_push_window_omni(), prv_push_window_classic());
}

static void prv_init(void) {
  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);
  
  push_proper_dial_window();
}

static void prv_deinit(void) {
  window_destroy(s_window_classic);
  window_destroy(s_window_omni);

  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  accel_tap_service_unsubscribe();
}

int main(void) {
  prv_init();

  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window");

  app_event_loop();
  prv_deinit();
}
