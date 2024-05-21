#include <pebble.h>
#include "utils.h"

/* ---------- Settings ---------- */
ClaySettings s_settings;

void default_settings() {
  s_settings.DialMode = 'c';
  s_settings.HideUI = false;
  s_settings.DoColorOverride = false;
  s_settings.CustomColor = GColorWhite;
  s_settings.HighContrast = false;
  s_settings.DoWeather = false;
  s_settings.WeatherUnits = 'f';
}

void load_settings() {
  default_settings();
  persist_read_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

void save_settings() {
  persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

/* ---------- Weather Data ---------- */
static WeatherData s_weather;
static bool s_js_ready = false;

void load_weather() {
  s_weather.Condition = 0;
  s_weather.TemperatureKelvin = 0;
  s_weather.Timestamp = 0;
  persist_read_data(WEATHER_KEY, &s_weather, sizeof(s_weather));
}

void save_weather() {
  persist_write_data(WEATHER_KEY, &s_weather, sizeof(s_weather));
}

bool request_new_weather() {
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

bool has_saved_weather() {
  load_weather();
  time_t now_time = time(NULL);
  /* Weather either is timestamped for the past,
     the future, or 0 (nonexistent). If the
     timestamp is in the future, we have
     valid saved weather. */ 
  return !(now_time > s_weather.Timestamp);
}

/* ---------- Weather Icons ---------- */
enum weather_conditions_with_icons owm_condition_simplify(int owm_condition) {
    /* OpenWeatherMap's API has a ton of different values it
       can return as condition codes. This function takes an
       OWM condition code and squashes it down to match one
       of the icons we have. */
    /* https://openweathermap.org/weather-conditions */
    switch (owm_condition) { // ranged cases may not be supported in pebble's flavor of c
        case 200 ... 232: return RAIN; // Thunderstorm
        case 300 ... 321: return RAIN; // Drizzle
        case 500 ... 531: return RAIN; // Rain
        case 600 ... 602: return SNOW; // Snow
        case 611 ... 616: return SLEET; // Sleet, or Rain and Snow
        case 620 ... 622: return SNOW; // Shower snow
        case 701 ... 781: return FOG; // Any atmospheric condition
        case 800: return CLEAR; // Clear
        case 801 ... 802: return PARTLY_CLOUDY; // Few or Scattered Clouds
        case 803 ... 804: return CLOUDY; // Broken or Overcast Clouds

        default: return ERROR_GENERIC; // value is not an OWM condition
    }
}

static bool prv_is_day() {
    /* Used when un-abstracting day-night variants of
       certain icons */
    //TODO figure out if sun is up
    return true; // for debugging
}

ResHandle res_handle_for_weather(enum weather_conditions_with_icons state) {
    /* This function takes in a simplified condition enum, and
       returns the ResHandle for the corresponding icon */
    switch (state) {
        case CLEAR: return resource_get_handle(prv_is_day() ? RESOURCE_ID_W_CLEAR_DAY : RESOURCE_ID_W_CLEAR_NIGHT); break;
        case CLOUDY: return resource_get_handle(RESOURCE_ID_W_CLOUDY); break;
        case FOG: return resource_get_handle(RESOURCE_ID_W_FOG); break;
        case PARTLY_CLOUDY: return resource_get_handle(prv_is_day() ? RESOURCE_ID_W_PARTLY_CLOUDY_DAY : RESOURCE_ID_W_PARTLY_CLOUDY_NIGHT); break;
        case RAIN: return resource_get_handle(RESOURCE_ID_W_RAIN); break;
        case SLEET: return resource_get_handle(RESOURCE_ID_W_SLEET); break;
        case SNOW: return resource_get_handle(RESOURCE_ID_W_SNOW); break;
        case WIND: return resource_get_handle(RESOURCE_ID_W_WIND); break;

        case ERROR_NO_TOKEN: return resource_get_handle(RESOURCE_ID_W_CLOUDY); break; // add error PDCs later
        case ERROR_GENERIC: // fall through to default
        default: return resource_get_handle(RESOURCE_ID_W_SNOW); // add error PDCs later
    }
}

/* ---------- UI ---------- */


/* ---------- Event handlers ---------- */
