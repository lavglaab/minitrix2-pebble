#include "colorize_pdc.h"

// This code all from
// https://github.com/hellcp/cutiefur/blob/main/src/c/drawing.c
// - thank you so much for talking me through it!

typedef struct {
  GColor color;
} ColorCBContext;

static bool prv_draw_command_list_in_color_cb(GDrawCommand *command,
                                              uint32_t index, void *context) {
  ColorCBContext *color = context;
  gdraw_command_set_fill_color(command, color->color);
  gdraw_command_set_stroke_color(command, color->color);
  return true;
}

void draw_command_image_recolor(GDrawCommandImage *image, GColor color) {
  ColorCBContext ctx = {
      .color = color,
  };
  gdraw_command_list_iterate(gdraw_command_image_get_command_list(image),
                             prv_draw_command_list_in_color_cb, &ctx);
}
