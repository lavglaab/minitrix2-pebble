#include <pebble.h>

/* ---------- Settings ---------- */
#define SETTINGS_KEY 1
#define VERSION_KEY 2

#define SETTINGS_VERSION_CURRENT 3 // 1: add RectClassicClockLtR, refactor, introduce versioning
                                   // 2: add DoColorBacklight
                                   // 3: add platform-specific battery threshold

typedef struct ClaySettings{
    char DialMode;

    bool HideUI;
    bool DoColorOverride;
    GColor CustomColor;
    bool HighContrast;

    bool DoWeather;
    char WeatherUnits;

    bool RectClassicClockLtR;
    bool DoColorBacklight;

    int LowBatterySelf;
    int LowBattery30D;
    int LowBattery14D;
    int LowBattery7D;
    int LowBattery2D;
} __attribute__((__packed__)) ClaySettings;

ClaySettings * settings_get();
void settings_deinit();
int settings_process_appmessage(DictionaryIterator *iter, void *context);
