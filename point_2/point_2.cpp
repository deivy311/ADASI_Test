

#include <gst/gst.h>
//#include <gst/rtsp-server/rtsp-server.h>
#include <gst/video/video.h>

#include <cairo.h>
#include <cairo-gobject.h>

#include <glib.h>

static gboolean
on_message (GstBus * bus, GstMessage * message, gpointer user_data)
{
  GMainLoop *loop = (GMainLoop *) user_data;

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err = NULL;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
      g_critical ("Got ERROR: %s (%s)", err->message, GST_STR_NULL (debug));
      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_WARNING:{
      GError *err = NULL;
      gchar *debug;

      gst_message_parse_warning (message, &err, &debug);
      g_warning ("Got WARNING: %s (%s)", err->message, GST_STR_NULL (debug));
      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_EOS:
      g_main_loop_quit (loop);
      break;
    default:
      break;
  }

  return TRUE;
}

/* Datastructure to share the state we are interested in between
 * prepare and render function. */
typedef struct
{
  gboolean valid;
  GstVideoInfo vinfo;
} CairoOverlayState;

/* Store the information from the caps that we are interested in. */
static void
prepare_overlay (GstElement * overlay, GstCaps * caps, gpointer user_data)
{
  CairoOverlayState *state = (CairoOverlayState *) user_data;

  state->valid = gst_video_info_from_caps (&state->vinfo, caps);
}

/* Draw the overlay. 
 * This function draws a cute "beating" heart. */
static void
draw_overlay (GstElement * overlay, cairo_t * cr, guint64 timestamp,
    guint64 duration, gpointer user_data)
{
  CairoOverlayState *s = (CairoOverlayState *) user_data;
  double scale;
  int width, height;

  if (!s->valid)
    return;

  width = GST_VIDEO_INFO_WIDTH (&s->vinfo);
  height = GST_VIDEO_INFO_HEIGHT (&s->vinfo);

  scale = 2 * (((timestamp / (int) 1e7) % 70) + 30) / 100.0;
  cairo_translate (cr, width / 2, (height / 2) - 30);

  /* FIXME: this assumes a pixel-aspect-ratio of 1/1 */
  cairo_scale (cr, scale, scale);

  cairo_move_to (cr, 0, 0);
  cairo_curve_to (cr, 0, -30, -50, -30, -50, 0);
  cairo_curve_to (cr, -50, 30, 0, 35, 0, 60);
  cairo_curve_to (cr, 0, 35, 50, 30, 50, 0);
  cairo_curve_to (cr, 50, -30, 0, -30, 0, 0);
  cairo_set_source_rgba (cr, 0.9, 0.0, 0.1, 0.7);
  cairo_fill (cr);
}
static GstElement *
setup_gst_pipeline (CairoOverlayState * overlay_state)
{
  GstElement *cairo_overlay;
  GstElement *source, *adaptor1, *adaptor2;
  GstElement* pipeline, * src, * overlay,*capsfilter,*videoconvert,* enc, * pay, * sink;

  /* Create elements */
    pipeline = gst_pipeline_new("mypipeline");
    src = gst_element_factory_make("videotestsrc", "mysrc");
    overlay = gst_element_factory_make("textoverlay", "myoverlay");
    // overlay = gst_element_factory_make ("cairooverlay", "overlay");
    capsfilter = gst_element_factory_make ("capsfilter", "capsfilter");
    videoconvert = gst_element_factory_make ("videoconvert", "videoconvert");
    enc = gst_element_factory_make("x264enc", "myenc");
    pay = gst_element_factory_make("rtph264pay", "mypay");
    sink = gst_element_factory_make("udpsink", "mysink");

    /* Set properties */
    g_object_set (G_OBJECT (capsfilter), "caps",
                  gst_caps_from_string ("video/x-raw, format=RGB"),
                  NULL);
    /* Set properties */
    g_object_set(G_OBJECT(pay), "config-interval", 10, NULL);
    g_object_set(G_OBJECT(pay), "pt", 96, NULL);

    g_object_set(G_OBJECT(sink), "host", "127.0.0.1", NULL);
    g_object_set(G_OBJECT(sink), "port", 5000, NULL);

    /* Add elements to pipeline */
    gst_bin_add_many(GST_BIN(pipeline), src, overlay, capsfilter, videoconvert, enc, pay, sink, NULL);

    if (!gst_element_link_many(src, overlay, capsfilter, videoconvert,enc,pay, sink, NULL)) {
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
    //   Set text and font properties
    g_object_set(G_OBJECT(overlay), "text", "Hello, world!", NULL);
    g_object_set(G_OBJECT(overlay), "font-desc", "Sans 24", NULL);

    // Set alignment and padding properties to move the text to the top-left corner
    g_object_set(G_OBJECT(overlay), "valignment", 0, "halignment", 0, "xpad", 50, "ypad", 50, NULL);

  return pipeline;
}
int main(int argc, char* argv[]) {
    GstElement* pipeline, * src, * overlay,* enc, * pay, * sink;
    GstCaps* caps;
    GstBus* bus;
    GstMessage* msg;
    GMainLoop* loop;
    CairoOverlayState *overlay_state;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

      /* allocate on heap for pedagogical reasons, makes code easier to transfer */
    overlay_state = g_new0 (CairoOverlayState, 1);
    pipeline = setup_gst_pipeline (overlay_state);

    /* Start playing */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Running...\n");
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_signal_watch (bus);
    g_signal_connect (G_OBJECT (bus), "message", G_CALLBACK (on_message), loop);
    gst_object_unref (GST_OBJECT (bus));

    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);

    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    g_free (overlay_state);

    return 0;
}