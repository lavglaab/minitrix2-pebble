#include <pebble.h>
#pragma once

#define SETTINGS_KEY 1

typedef struct ClaySettings{
    char DialMode;
    
    bool HideUI;
    bool DoColorOverride;
    GColor CustomColor;
    bool HighContrast;
    
    bool DoWeather;
} __attribute__((__packed__)) ClaySettings;