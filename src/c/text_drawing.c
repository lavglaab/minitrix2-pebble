#include "text_drawing.h"
#include "debug_flags.h"

GFont text_get_complication_font() {
    PreferredContentSize content_size = preferred_content_size();
    switch (content_size) {
        case PreferredContentSizeExtraLarge: // usually hidpi && large
        return fonts_get_system_font(FONT_KEY_GOTHIC_28);
            break;
        case PreferredContentSizeLarge: // usually (hidpi && medium) || (regulardpi && large)
            return fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
            break;
        case PreferredContentSizeMedium: // usually (hidpi && small) || (regulardpi && medium)
            // fall through, smallest size is 18
        case PreferredContentSizeSmall: // usually regulardpi && small
            // fall through, smallest size is 18
        default:
            return fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
            break;
    }
}

GSize text_measure_simply(const char * text, GFont const font, GRect box_max) {
    return graphics_text_layout_get_content_size(
        text, font, box_max, GTextOverflowModeFill, GTextAlignmentCenter
    );
}

// Draws text centered horizontally and vertically around the given point
void text_draw_centered(GContext * ctx, const char * text, GFont const font, GPoint origin, int offset_factor) {
    #if defined(DEBUG_UI_TEXT_LOCATIONS)
    // Draw a target at our origin
    graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorWhite));
    graphics_draw_circle(ctx, origin, 10);
    #endif

    GRect box_max = GRect(0, 0, 200, 100); // unrealistically large constraint bounds, chosen arbitrarily

    // Figure out how large our text actually needs to be
    GSize size_measured = text_measure_simply(text, font, box_max);

    // Create the bounds we will draw our text within
    GRect bounds_adjusted = GRect(
        origin.x - (size_measured.w / 2),
        origin.y - (size_measured.h / 2) - (size_measured.h / offset_factor), // text lines on pebble are taller than the actual type itself
        size_measured.w,
        size_measured.h
    );

    // And actually draw the text to the ctx
    graphics_draw_text(ctx, text, font, bounds_adjusted, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

}
