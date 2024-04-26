#include <pebble.h>
#pragma once

#define WEATHER_KEY 2

typedef struct WeatherData{
    int TemperatureKelvin;
    int Condition; // see https://openweathermap.org/weather-conditions
    int Timestamp; // should be one hour after data was received
} __attribute__((__packed__)) WeatherData;