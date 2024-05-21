#include <pebble.h>
#pragma once

/* ---------- Settings ---------- */
#define SETTINGS_KEY 1

typedef struct ClaySettings{
    char DialMode;
    
    bool HideUI;
    bool DoColorOverride;
    GColor CustomColor;
    bool HighContrast;
    
    bool DoWeather;
    char WeatherUnits;
} __attribute__((__packed__)) ClaySettings;

static ClaySettings s_settings;

void default_settings();

void load_settings();

void save_settings();

/* ---------- Weather Data ---------- */
#define WEATHER_KEY 2

typedef struct WeatherData{
    int TemperatureKelvin;
    int Condition; // see https://openweathermap.org/weather-conditions
    int Timestamp; // should be one hour after data was received
} __attribute__((__packed__)) WeatherData;

static WeatherData s_weather;
static bool s_js_ready;

void load_weather();

void save_weather();

bool request_new_weather(); // Returns true on success or false on failure

bool has_saved_weather(); // Returns true if saved weather is valid

/* ---------- Weather Icons ---------- */
enum weather_conditions_with_icons {
	CLEAR, // day/night variants are abstracted
    CLOUDY,
    FOG,
    PARTLY_CLOUDY, // day-night variants are abstracted
    RAIN,
    SLEET,
    SNOW,
    WIND,

    ERROR_GENERIC, // Represent errors fetching data
    ERROR_NO_TOKEN // Represent user not providing a token
};

/* OpenWeatherMap's API has a ton of different values it
   can return as condition codes. This function takes an
   OWM condition code and squashes it down to match one
   of the icons we have. */
    /* https://openweathermap.org/weather-conditions */
enum weather_conditions_with_icons owm_condition_simplify(int owm_condition);

// Returns a PDC icon ResHandle for the corresponding weather condition
ResHandle res_handle_for_weather(enum weather_conditions_with_icons condition);

/* ---------- UI ---------- */

/* ---------- Event handlers ---------- */