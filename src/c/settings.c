#include <pebble.h>
#include "settings.h"

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
