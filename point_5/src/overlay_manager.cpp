#include "overlay_manager.h"

// define constructor and destructor
overlay_manager::overlay_manager()
{
    g_print("overlay_manager constructor called\n");
}


/* Store the information from the caps that we are interested in. */
void overlay_manager::prepare_overlay(GstElement *overlay, GstCaps *caps, gpointer user_data)
{
  CairoOverlayState *state = (CairoOverlayState *)user_data;

  state->valid = gst_video_info_from_caps(&state->vinfo, caps);
}

/* Draw the overlay.
 * This function draws a cute "beating" heart. */
void overlay_manager::draw_overlay(GstElement *overlay, cairo_t *cr, guint64 timestamp,
                         guint64 duration, gpointer user_data)
{
  CairoOverlayState *s = (CairoOverlayState *)user_data;
  double scale;
  int width, height;

  if (!s->valid)
    return;

  // Get the width and height of the video frame
  width = GST_VIDEO_INFO_WIDTH(&s->vinfo);
  height = GST_VIDEO_INFO_HEIGHT(&s->vinfo);

  // Calculate the scaling factor based on the timestamp
  scale = 2 * (((timestamp / (int)1e7) % 70) + 30) / 100.0;

  // Translate the coordinate system to the center of the video frame
  cairo_translate(cr, width / 2, (height / 2) - 30);

  // Scale the coordinate system by the scaling factor
  // FIXME: this assumes a pixel-aspect-ratio of 1/1
  cairo_scale(cr, scale, scale);

  // Draw a rectangle with rounded corners
  cairo_rectangle(cr, -50, -30, 100, 60);
  cairo_set_source_rgba(cr, 0.9, 0.0, 0.1, 0.7);
  cairo_fill(cr);

  // Set the font face, size, and color for the text
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 9);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

  // Move the text cursor to the center of the rectangle
  cairo_move_to(cr, -30, 10);

  // Display the text "ADASI Test" in the rectangle
  cairo_show_text(cr, "ADASI Test");
}