#include "scale_pdc.h"

typedef struct {
    float scale_factor;
} ScalePointCBContext;

static GPoint prv_scale_gpoint(GPoint point, float scale_factor) {
    GPoint scaled_point = GPoint(
        point.x *= scale_factor,
        point.y *= scale_factor
    );
    return scaled_point;
}

static bool prv_gdraw_command_scale_points(GDrawCommand *command, uint32_t index, void *context) {
  ScalePointCBContext *scale_ctx = context;
  for (int i = 0; i < gdraw_command_get_num_points(command); i++) {
    gdraw_command_set_point(command, i, prv_scale_gpoint(gdraw_command_get_point(command, i), scale_ctx->scale_factor));
  }
  return true;
}

void scale_draw_command_list(GDrawCommandList *list, float scale_factor) {
  ScalePointCBContext ctx = {
      .scale_factor = scale_factor,
  };
  gdraw_command_list_iterate(list, prv_gdraw_command_scale_points, &ctx);
}

void scale_draw_command_image(GDrawCommandImage *image, float scale_factor) {
    scale_draw_command_list(gdraw_command_image_get_command_list(image), scale_factor);
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

    scale_draw_command_image(image, scale_factor);
    gdraw_command_image_set_bounds_size(image, GSize(end_dimen, end_dimen));
}
