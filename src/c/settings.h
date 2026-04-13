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
} __attribute__((__packed__)) ClaySettings;

ClaySettings s_settings;

void default_settings();

void load_settings();

void save_settings();
