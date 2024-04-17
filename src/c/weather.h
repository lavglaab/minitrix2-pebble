#include <pebble.h>
#pragma once

#define WEATHER_KEY 2

typedef struct WeatherData{
    int ReturnCode; // where 0=OK, 1=Error, and 2=NoToken

    int Temperature;
    int Timestamp;
    char[] Condition;
} __attribute__((__packed__)) WeatherData;