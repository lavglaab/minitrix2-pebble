#include <pebble.h>

GPoint scale_gpoint(GPoint point, int kilo_scale_x, int kilo_scale_y);

void scale_draw_command_list(GDrawCommandList *list, int kilo_scale_factor);
void scale_draw_command_image(GDrawCommandImage *image, int kilo_scale_factor);

void draw_command_image_fill_size(GDrawCommandImage *image, GSize size);
