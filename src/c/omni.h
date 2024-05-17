#include <pebble.h>

/* ---------- Palette colors ----------*/
#define PAL_OMNI_STRIPES PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite)
#define PAL_OMNI_BG GColorBlack
#define PAL_OMNI_DATA PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack)

#define PAL_OMNI_STATUS_DISCONNECT PBL_IF_COLOR_ELSE(GColorYellow, GColorDarkGray)
#define PAL_OMNI_STATUS_LOWBATT PBL_IF_COLOR_ELSE(GColorRed, GColorDarkGray)

/* ---------- Text bounds ----------*/
#define BOUND_OMNI_TIME_H1 PBL_IF_RECT_ELSE(GRect(0,-10,36,56), GRect(21,14,36,56))
#define BOUND_OMNI_TIME_H2 PBL_IF_RECT_ELSE(GRect(24,31,36,56), GRect(43,46,36,56))
#define BOUND_OMNI_TIME_M1 PBL_IF_RECT_ELSE(GRect(83,61,36,56), GRect(101,68,36,56))
#define BOUND_OMNI_TIME_M2 PBL_IF_RECT_ELSE(GRect(108,102,36,56), GRect(123,98,36,56))

#define BOUND_OMNI_DATE PBL_IF_RECT_ELSE(GRect(3,116,48,48), GRect(21,110,48,48))
#define BOUND_OMNI_WEATHER PBL_IF_RECT_ELSE(GRect(93,4,48,48), GRect(109,14,48,48))
#define BOUND_OMNI_WEATHER_ICON PBL_IF_RECT_ELSE(GRect(112,12,25,25), GRect(126,26,25,25))