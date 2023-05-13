#include <iostream>
#include <gstreamer-1.0/gst/gst.h>
#include <glib.h>

using namespace std;

int main(int argc, char* argv[]) {

    GstElement *pipeline, *source, *overlay, *capsfilter, *videoconvert, *enc, *pay, *udpsink;
    GMainLoop *loop;

    /* Initialize GStreamer */
    gst_init (&argc, &argv);

    /* Create elements */
    pipeline = gst_pipeline_new ("my-pipeline");
    source = gst_element_factory_make ("videotestsrc", "source");
    overlay = gst_element_factory_make ("cairooverlay", "overlay");
    capsfilter = gst_element_factory_make ("capsfilter", "capsfilter");
    videoconvert = gst_element_factory_make ("videoconvert", "videoconvert");
    enc = gst_element_factory_make ("x264enc", "enc");
    pay = gst_element_factory_make ("rtph264pay", "pay");
    udpsink = gst_element_factory_make ("udpsink", "udpsink");

    /* Set properties */
    g_object_set (G_OBJECT (capsfilter), "caps",
                  gst_caps_from_string ("video/x-raw, format=RGB"),
                  NULL);
    g_object_set (G_OBJECT (udpsink), "host", "127.0.0.1",
                  "port", 5000, NULL);

    /* Add elements to pipeline */
    gst_bin_add_many (GST_BIN (pipeline), source, overlay, capsfilter, videoconvert, enc, pay, udpsink, NULL);

    /* Link elements */
    gst_element_link_many (source, overlay, capsfilter, videoconvert, enc, pay, udpsink, NULL);

    /* Set up the overlay */
    cairo_t *cr = gst_cairo_overlay_get_context(GST_CAIRO_OVERLAY(overlay));
    cairo_set_source_rgb(cr, 1.0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 30.0);
    cairo_move_to(cr, 10.0, 50.0);
    cairo_show_text(cr, "Hello, world!");
    cairo_destroy(cr);

    /* Start streaming */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* Run main loop */
    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    /* Clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);

    return 0;
}
