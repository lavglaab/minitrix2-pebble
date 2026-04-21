#include <pebble.h>
#include "settings.h"
#include "weather.h"
#include "windows/classic.h"
#include "windows/omni.h"

#include "debug_flags.h"

// Convenience macro for doing stuff based on dial mode
#define UI_IF_OMNI_ELSE(a, b) (DEBUG_UI_ALWAYS_OV || settings_get()->DialMode == 'o') ? (a) : (b)

static Window *s_window_classic;
static Window *s_window_omni;

static AppTimer *s_tap_timer;

/* ---------- Message handling ---------- */

static void push_proper_dial_window(); // Defined under Lifecycle

static void common_update_weather(int response_code); // defined under UI Common
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  /* ---- Comms state ---- */
  Tuple *js_ready_t = dict_find(iter, MESSAGE_KEY_JSReady);
  if(js_ready_t && settings_get()->DoWeather) {
    s_js_ready = true;
    LOG_IF_ENABLED(DEBUG_LOG_WEATHER, APP_LOG_LEVEL_INFO, "JS is ready! Asking for weather");
    request_new_weather();
  }

  /* ---- Settings ---- */
  int message_contains_settings = settings_process_appmessage(iter, context);
  if(message_contains_settings != 0) {
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
        s_weather.Timestamp = now_time + (60 * (1 * 60)); // Set timestamp for one hour from now
        save_weather();
        }
        break;
      case 1:
        // Some kind of error fetching data
        break;
      case 2:
        // User hasn't provided a token
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
  if ((changed & HOUR_UNIT) != 0 && settings_get()->DoWeather && !has_saved_weather()) { request_new_weather(); }
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
  if (settings_get()->HideUI) {
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

const bool animated = false;

static void prv_push_window_classic() {
  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_DEBUG, "prv_push_window_classic");
  if (s_window_classic != NULL) {
      LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Classic window already exists");
      if (window_stack_get_top_window() == s_window_classic) {
          LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Classic window already on top, updating style");
          common_update_style();
          return;
      }
      if (window_is_loaded(s_window_classic)) {
          LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Classic window is already loaded, pushing");
          window_stack_push(s_window_classic, animated);
          return;
      }

      LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Destroying classic window");
      window_destroy(s_window_classic);
      s_window_classic = NULL;
  }

  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Creating classic window");
  s_window_classic = window_create();
  window_set_window_handlers(s_window_classic, (WindowHandlers) {
    .load = classic_window_load,
    .unload = classic_window_unload,
  });
  window_stack_push(s_window_classic, animated);
}

static void prv_push_window_omni() {
  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_DEBUG, "prv_push_window_omni");
  if (s_window_omni != NULL) {
      LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "OV window already exists");
      if (window_stack_get_top_window() == s_window_omni) {
          LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "OV window already on top, updating style");
          common_update_style();
          return;
      }
      if (window_is_loaded(s_window_omni)) {
          LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "OV window is already loaded, pushing");
          window_stack_push(s_window_omni, animated);
          return;
      }

      LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Destroying OV window");
      window_destroy(s_window_omni);
      s_window_omni = NULL;
  }

  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Creating omni window");
  s_window_omni = window_create();
  window_set_window_handlers(s_window_omni, (WindowHandlers) {
    .load = omni_window_load,
    .unload = omni_window_unload,
  });
  window_stack_push(s_window_omni, animated);
}

static void prv_window_load_common() {
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
  if (settings_get()->HideUI) { accel_tap_service_subscribe(accel_tap_handler); }
}

static void push_proper_dial_window() {
  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_DEBUG, "push_proper_dial_window");

  prv_window_load_common();

  if (UI_IF_OMNI_ELSE(true, false)) {
      // omniverse dial path
      LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "OV dial window codepath");

      prv_push_window_omni();

      if (window_stack_contains_window(s_window_classic)) {
          LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Classic window in window stack, removing");
          window_stack_remove(s_window_classic, animated);
          LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Destroying classic window");
          window_destroy(s_window_classic);
          s_window_classic = NULL;
      }
  } else {
      // classic/uaf dial path
      LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Classic dial window codepath");

      prv_push_window_classic();

      if (window_stack_contains_window(s_window_omni)) {
          LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Omni window in window stack, removing");
          window_stack_remove(s_window_omni, animated);
          LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "Destroying omni window");
          window_destroy(s_window_omni);
          s_window_omni = NULL;
      }
  }

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

  settings_deinit();
}

int main(void) {
  prv_init();

  LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window");

  app_event_loop();
  prv_deinit();
}
