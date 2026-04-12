#include "scale_pdc.h"

// static float s_scale_factor;

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
