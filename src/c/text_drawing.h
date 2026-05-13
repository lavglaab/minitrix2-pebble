#include <pebble.h>
GFont text_get_complication_font();
GSize text_measure_simply(const char * text, GFont const font, GRect box_max);
void text_draw_centered(GContext * ctx, const char * text, GFont const font, GPoint origin);
