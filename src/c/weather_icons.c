#include <pebble.h>

enum prv_states_with_icons {
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

static enum prv_states_with_icons prv_owm_condition_simplify(int condition) {
    /* OpenWeatherMap's API has a ton of different values it
       can return as condition codes. This function takes an
       OWM condition code and squashes it down to match one
       of the icons we have. */
    /* https://openweathermap.org/weather-conditions */
    switch (condition) { // ranged cases may not be supported in pebble's flavor of c
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

        default: return ERROR_GENERIC; // this is not an OWM condition
    }
}

static bool prv_is_day() {
    /* Used when un-abstracting day-night variants of
       certain icons */
    //TODO figure out if sun is up
    return true; // for debugging
}

static ResHandle res_handle_for_weather(enum prv_states_with_icons state) {
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