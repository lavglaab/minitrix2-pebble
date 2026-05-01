#include <pebble.h>

/* ---------- UI ---------- */
void classic_update_minute();
void classic_update_date();
void classic_update_weather(int response_code);
void classic_update_style();
void classic_ui_set_hidden(bool value);

/* ----------Life cycle ---------- */
void classic_window_load(Window *window);
void classic_window_unload(Window *window);

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
// #if defined(PBL_RECT)
#define POINT_CLASSIC_TIME_RECT_X_SCL 15 // what are these fucking names, lav, what are we doing here
#define POINT_CLASSIC_TIME_RECT_Y_SCL 35

#define POINT_CLASSIC_TIME_TOP_LEFT GPoint(12, 34)
#define POINT_CLASSIC_TIME_BOTTOM_LEFT GPoint(15, 100-35)
#define POINT_CLASSIC_TIME_TOP_RIGHT GPoint(100-15, 35)
#define POINT_CLASSIC_TIME_BOTTOM_RIGHT GPoint(100-15, 100-35)
// #endif

#if defined(PBL_ROUND)
#define POINT_CLASSIC_TIME_ROUND_X_SCL 18
#define POINT_CLASSIC_TIME_ROUND_Y_SCL 50
#endif

#define BOUND_CLASSIC_DATE PBL_IF_RECT_ELSE(GRect(0,0,144,20), GRect(0,8,180,48))
#define BOUND_CLASSIC_WEATHER PBL_IF_RECT_ELSE(GRect(0,144,144,20), GRect(0,116,180,48))
#define BOUND_CLASSIC_WEATHER_ICON PBL_IF_RECT_ELSE(GRect(50,139,25,25), GRect(68,145,25,25))
