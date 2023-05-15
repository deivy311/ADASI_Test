#include <gst/gst.h>
#include <glib-unix.h>
#include <gst/video/video.h>

#include <cairo.h>
#include <cairo-gobject.h>

#include <glib.h>
#define HOST "127.0.0.1"

gboolean signal_handler (gpointer user_data)
{
  GMainLoop *loop = (GMainLoop *) user_data;

  g_print("closing the main loop");
  g_main_loop_quit(loop);

  return TRUE;
}

int main (gint argc, gchar *argv[])
{
  GstElement *pipeline;
  GstElement *src;
  GstElement *decoder;
  GstElement *videoconvert;
  GstElement *videoscale;
  GstElement *encoder;
  GstElement *capsfilter;
  GstElement *muxer;
  GstElement *sink,*overlay;
  GstCaps *caps;
  GMainLoop *loop;
  gint ret = -1;

  gst_init(NULL, NULL);

  pipeline = gst_pipeline_new("pipeline");

  src = gst_element_factory_make("videotestsrc", "mysrc");
  // overlay = gst_element_factory_make("textoverlay", "myoverlay");
    overlay = gst_element_factory_make ("cairooverlay", "myoverlay");

  // decoder = gst_element_factory_make("decodebin", "decoder");
  videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
  videoscale = gst_element_factory_make("videoscale", "videoscale");
  capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
  caps = gst_caps_from_string("video/x-raw,width=640,height=480");
  g_object_set(capsfilter, "caps", caps, NULL);
  gst_caps_unref(caps);
  encoder = gst_element_factory_make("x264enc", "x264enc");
  muxer = gst_element_factory_make("matroskamux", "matroskamux");
  sink = gst_element_factory_make("tcpserversink", "tcpserversink");

  g_object_set(sink, "host", HOST, "port", 5002, NULL);

  gst_bin_add_many(GST_BIN(pipeline), src, overlay, videoconvert, videoscale, capsfilter, encoder, muxer, sink, NULL);
    if (!gst_element_link_many(src, overlay, videoconvert, videoscale, capsfilter, encoder, muxer, sink, NULL))
    {
        g_printerr("Failed to link elements\n");
        g_warning ("Failed to link elements!");
    }
    // g_object_set(G_OBJECT(overlay), "draw", [](cairo_t *cr, int width, int height, gpointer data) {
    //     // Draw overlay here
    //     cairo_set_source_rgb(cr, 1, 1, 1);
    //     cairo_rectangle(cr, 10, 10, 100, 50);
    //     cairo_fill(cr);
    // }, NULL, NULL);
    // g_object_set(G_OBJECT(overlay), "text", "Hello, world!", NULL);
  // // g_object_set(src, "location", "../Files/video_test_2.mp4", NULL);
  //   g_object_set(G_OBJECT(overlay), "text", "Hello, world!", NULL);
  //   g_object_set(G_OBJECT(overlay), "font-desc", "Sans 24", NULL);

  //   // Set alignment and padding properties to move the text to the top-left corner
  //   g_object_set(G_OBJECT(overlay), "valignment", 0, "halignment", 0, "xpad", 50, "ypad", 50, NULL);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_print("Pipeline playing\n");

  loop = g_main_loop_new(NULL, TRUE);
  g_unix_signal_add(SIGINT, signal_handler, loop);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  gst_element_set_state(pipeline, GST_STATE_NULL);
  g_print("Closed successfully\n");

  ret = 0;
  goto no_src;
no_src:
  gst_object_unref(pipeline);
  gst_deinit();

  return ret;
}
