#include <gst/gst.h>
#include <gst/video/video.h>
#include <cairo.h>
#include <cairo-gobject.h>
#include <glib.h>
#include <gst/rtsp-server/rtsp-server.h>

#define DEFAULT_RTSP_PORT "5001"
#define VIDEO_WIDTH 720
#define VIDEO_HEIGHT 480
#define VIDEO_FPS 50
static char* port = (char*)DEFAULT_RTSP_PORT;
using namespace std;
static GOptionEntry entries[] = {
  {"port", 'p', 0, G_OPTION_ARG_STRING, &port,
      "Port to listen on (default: " DEFAULT_RTSP_PORT ")", "PORT"},
  {NULL}
};

/* called when a stream has received an RTCP packet from the client */
static void
on_ssrc_active(GObject* session, GObject* source, GstRTSPMedia* media)
{
    GstStructure* stats;

    GST_INFO("source %p in session %p is active", source, session);

    g_object_get(source, "stats", &stats, NULL);
    if (stats) {
        gchar* sstr;

        sstr = gst_structure_to_string(stats);
        g_print("structure: %s\n", sstr);
        g_free(sstr);

        gst_structure_free(stats);
    }
}

static void
on_sender_ssrc_active(GObject* session, GObject* source,
    GstRTSPMedia* media)
{
    GstStructure* stats;

    GST_INFO("source %p in session %p is active", source, session);

    g_object_get(source, "stats", &stats, NULL);
    if (stats) {
        gchar* sstr;

        sstr = gst_structure_to_string(stats);
        g_print("Sender stats:\nstructure: %s\n", sstr);
        g_free(sstr);

        gst_structure_free(stats);
    }
}

/* signal callback when the media is prepared for streaming. We can get the
 * session manager for each of the streams and connect to some signals. */
static void
media_prepared_cb(GstRTSPMedia* media)
{
    guint i, n_streams;

    n_streams = gst_rtsp_media_n_streams(media);

    GST_INFO("media %p is prepared and has %u streams", media, n_streams);

    for (i = 0; i < n_streams; i++) {
        GstRTSPStream* stream;
        GObject* session;

        stream = gst_rtsp_media_get_stream(media, i);
        if (stream == NULL)
            continue;

        session = gst_rtsp_stream_get_rtpsession(stream);
        GST_INFO("watching session %p on stream %u", session, i);

        g_signal_connect(session, "on-ssrc-active",
            (GCallback)on_ssrc_active, media);
        g_signal_connect(session, "on-sender-ssrc-active",
            (GCallback)on_sender_ssrc_active, media);
    }
}
static void
media_configure_cb(GstRTSPMediaFactory* factory, GstRTSPMedia* media)
{
  /* connect our prepared signal so that we can see when this media is
   * prepared for streaming */
  g_signal_connect(media, "prepared", (GCallback)media_prepared_cb, factory);
}
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
  GstElement *adaptor1, *adaptor2;
  GstElement* pipeline, * src, * overlay,*capsfilter,*videoconvert, * pay, * sink;
  gchar* str;
  GstRTSPMediaFactory* factory;
  GstRTSPServer* server;
  GstRTSPMountPoints* mounts;
  GstElement *videoscale;
  GstElement *encoder;
  GstElement *muxer;
  GstCaps *caps;
  GMainLoop *loop;
  

  /* Create elements */
    pipeline = gst_pipeline_new("mypipeline");
  src = gst_element_factory_make("autovideosrc", "autovideosrc");
    adaptor1 = gst_element_factory_make ("videoconvert", "adaptor1");
    overlay = gst_element_factory_make ("cairooverlay", "myoverlay");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    videoscale = gst_element_factory_make("videoscale", "videoscale");
    capsfilter = gst_element_factory_make("capsfilter", "capsfilter");

    caps = gst_caps_from_string("video/x-raw,width=640,height=480");
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);
    encoder = gst_element_factory_make("x264enc", "x264enc");
    muxer = gst_element_factory_make("matroskamux", "matroskamux");
    sink = gst_element_factory_make("tcpserversink", "tcpserversink");

    server = gst_rtsp_server_new();
    g_object_set(server, "service", port, NULL);

    mounts = gst_rtsp_server_get_mount_points(server);
    str = g_strdup_printf("( "
    "filesrc location=\"%s\" ! qtdemux name=d "
    "d. ! queue ! rtph264pay pt=96 name=pay0 "
    "d. ! queue ! rtpmp4apay pt=97 name=pay1 " ")", "../Files/video_test_2.mp4");

    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, str);
    // gst_rtsp_media_factory_set_shared(factory, TRUE);
    g_signal_connect(factory, "media-configure", (GCallback)media_configure_cb,
    factory);
    g_free(str);
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_object_unref(mounts);

    /* attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);

    g_object_set(G_OBJECT(sink), "host", "127.0.0.1", NULL);
    g_object_set(G_OBJECT(sink), "port", 5002, NULL);

    /* Add elements to pipeline */

    gst_bin_add_many(GST_BIN(pipeline), src,adaptor1, overlay, videoconvert, videoscale, capsfilter, encoder, muxer, sink, NULL);
    if (!gst_element_link_many(src,adaptor1, overlay, videoconvert, videoscale, capsfilter, encoder, muxer, sink, NULL))
    {
        g_printerr("Failed to link elements\n");
        g_warning ("Failed to link elements!");
    }
  /* Hook up the necessary signals for cairooverlay */
    g_signal_connect (overlay, "draw",
      G_CALLBACK (draw_overlay), overlay_state);
    g_signal_connect (overlay, "caps-changed",
      G_CALLBACK (prepare_overlay), overlay_state);

  return pipeline;
}

int main(int argc, char* argv[]) {
  GstElement* pipeline;  // Declare GStreamer pipeline
  GstBus* bus;           // Declare GStreamer bus
  GMainLoop* loop;       // Declare GMainLoop
  CairoOverlayState* overlay_state;  // Declare Cairo overlay state struct

  /* Initialize GStreamer */
  gst_init(&argc, &argv);

  /* Create a new GMainLoop */
  loop = g_main_loop_new(NULL, FALSE);

  /* Allocate memory for Cairo overlay state struct on heap */
  overlay_state = g_new0(CairoOverlayState, 1);

  /* Set up the GStreamer pipeline */
  pipeline = setup_gst_pipeline(overlay_state);

  /* Start playing the pipeline */
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_print("Running...\n");

  /* Get the pipeline's bus */
  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));

  /* Add a signal watch for the bus */
  gst_bus_add_signal_watch(bus);

  /* Connect the bus's "message" signal to the on_message callback function */
  g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(on_message), loop);

  /* Unreference the bus */
  gst_object_unref(GST_OBJECT(bus));

  /* Run the GMainLoop */
  g_main_loop_run(loop);

  /* Stop the pipeline */
  gst_element_set_state(pipeline, GST_STATE_NULL);

  /* Unreference the pipeline */
  gst_object_unref(pipeline);

  /* Free the Cairo overlay state struct */
  g_free(overlay_state);

  return 0;
}
