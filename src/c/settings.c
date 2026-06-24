#include "settings.h"
#include <pebble.h>
#include "debug_flags.h"

/* ---------- Settings ---------- */
static ClaySettings *s_settings;

static ClaySettings s_default_settings = {
    .DialMode = 'c',
    .HideUI = false,
    .DoColorOverride = false,
    .CustomColor = GColorWhite,
    .HighContrast = false,
    .DoWeather = false,
    .WeatherUnits = 'f',
    .RectClassicClockLtR = true
};

static void prv_settings_save() {
    // APP_LOG(APP_LOG_LEVEL_INFO, "persist write settings");
    persist_write_data(SETTINGS_KEY, s_settings, sizeof(ClaySettings));
}

static int prv_settings_migrate() {
    int statuscode = 0; // 0 == no need to migrate settings
                        // 1 == OK, did a settings migration

    // minitrix2 originally did not version its saved settings, so if we have settings with no version, assume it was the original defaults
    // between version 0.2 and whatever comes after, i plan to add a new setting for text flow direction on the rect classic dial. The default behavior will be different than the previous default behavior, so i want to apply the old behavior to existing user prefs, and the new behavior to new prefs

    // so i think what we want to actually do is
    // - check if persist exists. if not, this is first run. if so, continue with migration
    // - check if persist contains a value for SettignsVersion. if not, we are 0.2->0.3
    // - if we are 0.2->0.3, set a value for classic-time-direction, and set settingsVersion = 1
    if (!persist_exists(VERSION_KEY) && !persist_exists(SETTINGS_KEY)) {
        // no persist exists, this is first run (or user is updated from 0.2 without ever touching settings)
        persist_write_int(VERSION_KEY, SETTINGS_VERSION_CURRENT);
        return statuscode;
    }
    if (!persist_exists(VERSION_KEY) && persist_exists(SETTINGS_KEY)) {
        // we have settings, but no version, which means we are coming from 0.2 settings
        statuscode = 1;
        persist_write_int(VERSION_KEY, 1);
        // TODO: set classic text flow direction pref to top->bottom
        settings_get()->RectClassicClockLtR = false;
        APP_LOG(APP_LOG_LEVEL_INFO, "Did settings migration: RectClassicClockLtR = false");
    }

    if (statuscode == 1) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Did settings migration");
        prv_settings_save();
    }
    return statuscode;
}

static void prv_settings_init() {
    if (s_settings == NULL) {
        // Allocate memory for our settings struct
        s_settings = malloc(sizeof(ClaySettings));
        LOG_IF_ENABLED(DEBUG_LOG_MEMORY, APP_LOG_LEVEL_INFO, "malloc s_settings");
    }

    // Prefill with default settings
    *s_settings = s_default_settings;
    // Apply saved prefs over defaults
    persist_read_data(SETTINGS_KEY, s_settings, sizeof(ClaySettings));
    // apply any necessary format changes across watchapp updates
    prv_settings_migrate();
}

ClaySettings * settings_get() {
    // LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "settings_get()");
    if (s_settings == NULL) {
        // allocate memory and create the pointer
        prv_settings_init();
    }
    // LOG_IF_ENABLED(DEBUG_LOG_LIFECYCLE, APP_LOG_LEVEL_INFO, "return s_settings");
    return s_settings;
}

void settings_deinit() {
    if (s_settings == NULL) { return; }
    // TODO: deallocate memory
    free(s_settings);
    s_settings = NULL;
    LOG_IF_ENABLED(DEBUG_LOG_MEMORY, APP_LOG_LEVEL_INFO, "free s_settings");
}

int settings_process_appmessage(DictionaryIterator *iter, void *context) { // Call this from our AppMessage received handler
    int statuscode = 0; // 0 == no settings received in this appmessage
                        // 1 == yes, this appmessage contains settings, and we've processed them
                        // -1 == some kind of error

    Tuple *dial_mode_t = dict_find(iter, MESSAGE_KEY_PrefDialMode);
    if(dial_mode_t) {
      settings_get()->DialMode = dial_mode_t->value->int8;
      statuscode = 1;
    }

    Tuple *hide_ui_t = dict_find(iter, MESSAGE_KEY_PrefHideUI);
    if(hide_ui_t) {
        settings_get()->HideUI = hide_ui_t->value->int32 == 1;
        statuscode = 1;
    }

    Tuple *clock_dir_t = dict_find(iter, MESSAGE_KEY_PrefRectClassicClockLtR);
    if(clock_dir_t) {
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "MESSAGE_KEY_PrefRectClassicClockLtR: %d", clock_dir_t->value->int8);
        settings_get()->RectClassicClockLtR = ((char)clock_dir_t->value->int8) == 't'; // this pref is passed as a char because i wanted a radiogroup in clay
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "settings_get()->RectClassicClockLtR: %d", settings_get()->RectClassicClockLtR);
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "clock_dir_t->value->int8 == 116: %d", (clock_dir_t->value->int8 == 116 /*'t'*/));
        statuscode = 1;
    }

    Tuple *high_contrast_t = dict_find(iter, MESSAGE_KEY_PrefHighContrast);
    if(high_contrast_t) {
        settings_get()->HighContrast = high_contrast_t->value->int32 == 1;
        statuscode = 1;
    }

    Tuple *do_color_override_t = dict_find(iter, MESSAGE_KEY_PrefDoColorOverride);
    if(do_color_override_t) {
        settings_get()->DoColorOverride = do_color_override_t->value->int32 == 1;
        statuscode = 1;
    }

    Tuple *custom_color_t = dict_find(iter, MESSAGE_KEY_PrefOverrideColor);
    if(custom_color_t) {
        settings_get()->CustomColor = GColorFromHEX(custom_color_t->value->int32);
        statuscode = 1;
    }

    //Features
    Tuple *do_weather_t = dict_find(iter, MESSAGE_KEY_PrefDoWeather);
    if(do_weather_t) {
        settings_get()->DoWeather = do_weather_t->value->int32 == 1;
        statuscode = 1;
    }

    Tuple *weather_units_t = dict_find(iter, MESSAGE_KEY_PrefWeatherUnits);
    if(weather_units_t) {
      settings_get()->WeatherUnits = weather_units_t->value->int8;
      statuscode = 1;
    }

    if (statuscode == 1) { prv_settings_save(); }
    return statuscode;
}
