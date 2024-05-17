#include <pebble.h>

/* ---------- Palette colors ----------*/
#define PAL_CLASSIC_JEWEL PBL_IF_COLOR_ELSE(GColorBrightGreen, GColorWhite)
#define PAL_CLASSIC_COMPLICATIONS PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack)
#define PAL_CLASSIC_GREY_CARETS GColorDarkGray
#define PAL_CLASSIC_BLACK_CARETS GColorBlack
#define PAL_CLASSIC_CLOCK GColorWhite
#define PAL_CLASSIC_CLOCK_OUTLINE GColorBlack

#if defined(PBL_COLOR)
#define PAL_CLASSIC_STATUS_DISCONNECT GColorChromeYellow
#define PAL_CLASSIC_STATUS_LOWBATT GColorRed
#define PAL_CLASSIC_TEXT_DISCONNECT GColorArmyGreen
#define PAL_CLASSIC_TEXT_LOWBATT GColorBulgarianRose
#endif

/* ---------- Text bounds ----------*/
#if defined(PBL_RECT)
#define BOUND_CLASSIC_TIME_H1 GRect(0,14,36,60)
#define BOUND_CLASSIC_TIME_M1 GRect(0,74,36,60)
#define BOUND_CLASSIC_TIME_H2 GRect(108,14,36,60)
#define BOUND_CLASSIC_TIME_M2 GRect(108,74,36,60)
#endif

#if defined(PBL_ROUND)
#define BOUND_CLASSIC_TIME_HOUR GRect(0,49,72,60)
#define BOUND_CLASSIC_TIME_MINUTE GRect(108,49,72,60)
#endif

#define BOUND_CLASSIC_DATE PBL_IF_RECT_ELSE(GRect(0,0,144,20), GRect(0,8,180,48))
#define BOUND_CLASSIC_WEATHER PBL_IF_RECT_ELSE(GRect(0,144,144,20), GRect(0,116,180,48))
#define BOUND_CLASSIC_WEATHER_ICON PBL_IF_RECT_ELSE(GRect(50,139,25,25), GRect(68,145,25,25))