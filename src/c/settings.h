#include <pebble.h>

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

    // int SettingsVersion;
} __attribute__((__packed__)) ClaySettings;

ClaySettings * settings_get();
void settings_deinit();
int settings_process_appmessage(DictionaryIterator *iter, void *context);
