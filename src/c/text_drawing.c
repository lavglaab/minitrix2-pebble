#include "text_drawing.h"
#include "debug_flags.h"

// Draws text centered horizontally and vertically around the given point
void text_draw_centered(GContext * ctx, const char * text, GFont const font, GPoint origin) {
    #if defined(DEBUG_UI_TEXT_LOCATIONS)
    // Draw a target at our origin
    graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorWhite));
    graphics_draw_circle(ctx, origin, 10);
    #endif

    GRect box_max = GRect(0, 0, 200, 100); // unrealistically large constraint bounds, chosen arbitrarily

    // Figure out how large our text actually needs to be
    GSize size_measured = graphics_text_layout_get_content_size(
        text, font, box_max, GTextOverflowModeFill, GTextAlignmentCenter
    );

    // Create the bounds we will draw our text within
    GRect bounds_adjusted = GRect(
        origin.x - (size_measured.w / 2),
        origin.y - (size_measured.h / 2) - (size_measured.h / 5), // text lines on pebble are taller than the actual type itself
        size_measured.w,
        size_measured.h
    );

    // And actually draw the text to the ctx
    graphics_draw_text(ctx, text, font, bounds_adjusted, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

}
