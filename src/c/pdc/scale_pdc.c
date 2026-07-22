#include "scale_pdc.h"

#define KILO_SCALE 1000 // To avoid floating-point math on a Pebble, we
                        // multiply our ints by 1000, operate on them,
                        // and then divide them back down

typedef struct {
    int kilo_scale_factor;
} ScalePointCBContext;

GPoint scale_gpoint(GPoint point, int kilo_scale_x, int kilo_scale_y) {
    GPoint scaled_point = GPoint(
        point.x = (point.x * kilo_scale_x) / KILO_SCALE,
        point.y = (point.y * kilo_scale_y) / KILO_SCALE
    );
    return scaled_point;
}

static bool prv_gdraw_command_scale_points(GDrawCommand *command, uint32_t index, void *context) {
  ScalePointCBContext *scale_ctx = context;
  // Move points
  for (int i = 0; i < gdraw_command_get_num_points(command); i++) {
    gdraw_command_set_point(command, i, scale_gpoint(gdraw_command_get_point(command, i), scale_ctx->kilo_scale_factor, scale_ctx->kilo_scale_factor));
  }

  //Scale stroke width
  uint8_t sw = gdraw_command_get_stroke_width(command);
  if (sw != 0) { gdraw_command_set_stroke_width(command, (sw * scale_ctx->kilo_scale_factor) / KILO_SCALE); }
  return true;
}

void scale_draw_command_list(GDrawCommandList *list, int kilo_scale_factor) {
  ScalePointCBContext ctx = {
      .kilo_scale_factor = kilo_scale_factor,
  };
  gdraw_command_list_iterate(list, prv_gdraw_command_scale_points, &ctx);
}

void scale_draw_command_image(GDrawCommandImage *image, int kilo_scale_factor) {
    scale_draw_command_list(gdraw_command_image_get_command_list(image), kilo_scale_factor);
}

static int prv_min(int a, int b) {
    return (a <= b) ? a : b;
}
static int prv_max(int a, int b) {
    return (a >= b) ? a : b;
}

void draw_command_image_fill_size(GDrawCommandImage *image, GSize size) {
    GSize start_size = gdraw_command_image_get_bounds_size(image);
    int start_dimen = prv_max(start_size.w, start_size.h);
    int end_dimen = prv_max(size.w, size.h);
    // So we want to scale our pdc such that its smallest edge is as large as the target's largest edge
    float scale_factor = (float)end_dimen / (float)start_dimen; // these need to be floats, otherwise they will divide as ints and give an int result
    int kilo_scale_factor = scale_factor * KILO_SCALE;

    scale_draw_command_image(image, kilo_scale_factor);
    gdraw_command_image_set_bounds_size(image, GSize(end_dimen, end_dimen));
}
