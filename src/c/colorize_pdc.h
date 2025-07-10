#include <pebble.h>

void draw_command_image_in_color(GDrawCommandImage *image, GColor color);
void draw_pdc_colorized(GContext *ctx, int resource, GColor color,
                         GPoint origin);