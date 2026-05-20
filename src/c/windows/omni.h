#include <pebble.h>

/* ---------- UI ---------- */
void omni_update_minute();
void omni_update_date();
void omni_update_weather(int response_code);
void omni_update_style();
void omni_ui_set_hidden(bool value);

/* ----------Life cycle ---------- */
void omni_window_load(Window *window);
void omni_window_unload(Window *window);

/* ---------- Palette colors ----------*/
#define PAL_OMNI_STRIPES PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite)
#define PAL_OMNI_BG GColorBlack
#define PAL_OMNI_DATA PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack)

#define PAL_OMNI_STATUS_DISCONNECT PBL_IF_COLOR_ELSE(GColorYellow, GColorDarkGray)
#define PAL_OMNI_STATUS_LOWBATT PBL_IF_COLOR_ELSE(GColorRed, GColorDarkGray)

/* ---------- Text bounds ----------*/
#define POINT_OMNI_TIME_TOP_LEFT PBL_IF_ROUND_ELSE(GPoint(22, 24), GPoint(16, 22))
#define POINT_OMNI_TIME_SECOND_LEFT PBL_IF_ROUND_ELSE(GPoint(33, 44), GPoint(32, 44))

#define BOUND_OMNI_WEATHER PBL_IF_RECT_ELSE(GRect(93,4,48,48), GRect(109,14,48,48))
#define BOUND_OMNI_WEATHER_ICON PBL_IF_RECT_ELSE(GRect(112,12,25,25), GRect(126,26,25,25))
