#include <gst/gst.h>
#include <gst/video/video.h>
#include <cairo.h>
#include <cairo-gobject.h>
#include <glib.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <RTP_manager.h>
#include <RTSP_manager.h>
#define DEFAULT_RTSP_PORT "5001"
#define DEFAULT_RTP_PORT 5002
#define DEFAULT_FILE_PATH "../Files/video_test_2.mp4"
#define DEFAULT_RTSP_HOST "127.0.0.1"
#define DEFAULT_RTP_HOST "127.0.0.1"
#define VIDEO_WIDTH 720
#define VIDEO_HEIGHT 480
#define VIDEO_FPS 50
using namespace std;

// /* called when a stream has received an RTCP packet from the client */
// static void on_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media)
// {
//   // Declare a pointer to a GstStructure
//   GstStructure *stats;

//   // Print information about the active source and session
//   GST_INFO("source %p in session %p is active", source, session);

//   // Get the "stats" property from the source object and assign it to stats
//   g_object_get(source, "stats", &stats, NULL);
//   if (stats)
//   {
//     gchar *sstr;

//     // Convert the stats structure to a string and print it
//     sstr = gst_structure_to_string(stats);
//     g_print("structure: %s\n", sstr);

//     // Free the string and the stats structure
//     g_free(sstr);
//     gst_structure_free(stats);
//   }
// }

// static void
// on_sender_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media)
// {
//   GstStructure *stats;

//   // Log that the sender's SSRC is active
//   GST_INFO("source %p in session %p is active", source, session);

//   // Get the stats for the source object
//   g_object_get(source, "stats", &stats, NULL);
//   if (stats)
//   {
//     gchar *sstr;

//     // Convert the stats structure to a string
//     sstr = gst_structure_to_string(stats);

//     // Print the sender's stats along with its structure
//     g_print("Sender stats:\nstructure: %s\n", sstr);
//     g_free(sstr);

//     // Free the stats structure
//     gst_structure_free(stats);
//   }
// }

/* signal callback when the media is prepared for streaming. We can get the
 * session manager for each of the streams and connect to some signals. */
// static void
// media_prepared_cb(GstRTSPMedia *media)
// {
//   guint i, n_streams;

//   // Get the number of streams in the media
//   n_streams = gst_rtsp_media_n_streams(media);

//   // Log the media and number of streams
//   GST_INFO("media %p is prepared and has %u streams", media, n_streams);

//   // Iterate through each stream in the media
//   for (i = 0; i < n_streams; i++)
//   {
//     GstRTSPStream *stream;
//     GObject *session;

//     // Get the stream and skip if it is NULL
//     stream = gst_rtsp_media_get_stream(media, i);
//     if (stream == NULL)
//       continue;

//     // Get the session associated with the stream
//     session = gst_rtsp_stream_get_rtpsession(stream);

//     // Log the session and stream number being watched
//     GST_INFO("watching session %p on stream %u", session, i);

//     // Connect to the "on-ssrc-active" signal of the session and call the "on_ssrc_active" function with the media as the user_data parameter
//     g_signal_connect(session, "on-ssrc-active",
//                      (GCallback)on_ssrc_active, media);

//     // Connect to the "on-sender-ssrc-active" signal of the session and call the "on_sender_ssrc_active" function with the media as the user_data parameter
//     g_signal_connect(session, "on-sender-ssrc-active",
//                      (GCallback)on_sender_ssrc_active, media);
//   }
// }

// static void
// media_configure_cb(GstRTSPMediaFactory *factory, GstRTSPMedia *media)
// {
//   /* connect our prepared signal so that we can see when this media is
//    * prepared for streaming */
//   g_signal_connect(media, "prepared", (GCallback)media_prepared_cb, factory);
// }
static gboolean
on_message(GstBus *bus, GstMessage *message, gpointer user_data)
{
  GMainLoop *loop = (GMainLoop *)user_data;

  switch (GST_MESSAGE_TYPE(message))
  {
  case GST_MESSAGE_ERROR: // if error message is received
  {
    GError *err = NULL;
    gchar *debug;

    // parse the error message and get error and debug information
    gst_message_parse_error(message, &err, &debug);

    // print the error message and debug information
    g_critical("Got ERROR: %s (%s)", err->message, GST_STR_NULL(debug));

    // quit the main loop
    g_main_loop_quit(loop);
    break;
  }
  case GST_MESSAGE_WARNING: // if warning message is received
  {
    GError *err = NULL;
    gchar *debug;

    // parse the warning message and get error and debug information
    gst_message_parse_warning(message, &err, &debug);

    // print the warning message and debug information
    g_warning("Got WARNING: %s (%s)", err->message, GST_STR_NULL(debug));

    // quit the main loop
    g_main_loop_quit(loop);
    break;
  }
  case GST_MESSAGE_EOS: // if end-of-stream message is received
    // quit the main loop
    g_main_loop_quit(loop);
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
prepare_overlay(GstElement *overlay, GstCaps *caps, gpointer user_data)
{
  CairoOverlayState *state = (CairoOverlayState *)user_data;

  state->valid = gst_video_info_from_caps(&state->vinfo, caps);
}

/* Draw the overlay.
 * This function draws a cute "beating" heart. */
static void draw_overlay(GstElement *overlay, cairo_t *cr, guint64 timestamp,
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

static GstElement *setup_gst_pipeline(CairoOverlayState *overlay_state)
{
  /* Define variables */
  GstElement *cairo_overlay;
  GstElement *adaptor1, *adaptor2;
  GstElement *pipeline, *src, *overlay, *capsfilter, *videoconvert, *pay, *sink;
  GstRTSPMediaFactory *factory;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstElement *videoscale;
  GstElement *encoder;
  GstElement *muxer;
  GstCaps *caps;
  GMainLoop *loop;

  /* Create pipeline and elements */
  pipeline = gst_pipeline_new("mypipeline");
  src = gst_element_factory_make("autovideosrc", "autovideosrc");          // Creates element for video capture from a device
  adaptor1 = gst_element_factory_make("videoconvert", "adaptor1");         // Converts between various video formats
  overlay = gst_element_factory_make("cairooverlay", "myoverlay");         // Adds cairo drawing on top of the video
  videoconvert = gst_element_factory_make("videoconvert", "videoconvert"); // Converts between various video formats
  videoscale = gst_element_factory_make("videoscale", "videoscale");       // Scales the video frame
  capsfilter = gst_element_factory_make("capsfilter", "capsfilter");       // Limits the capabilities of the pipeline
  caps = gst_caps_from_string("video/x-raw,width=640,height=480");         // Set the resolution of the video
  g_object_set(capsfilter, "caps", caps, NULL);                            // Set the capsfilter to limit capabilities
  gst_caps_unref(caps);
  encoder = gst_element_factory_make("x264enc", "x264enc");          // Compresses video with the x264 codec
  muxer = gst_element_factory_make("matroskamux", "matroskamux");    // Muxes different streams of data into a Matroska file format
  sink = gst_element_factory_make("tcpserversink", "tcpserversink"); // Sends video data to the client over TCP
  RTSP_manager* rtsp_server_manager = new RTSP_manager(server, mounts, factory,DEFAULT_RTSP_HOST,DEFAULT_RTSP_PORT,DEFAULT_FILE_PATH);
  
  /* Set TCP server sink properties */
  g_object_set(G_OBJECT(sink), "host", DEFAULT_RTP_HOST, NULL); // Set the host IP

  // Set the "port" property of the "sink" element to 5002
  g_object_set(G_OBJECT(sink), "port", DEFAULT_RTP_PORT, NULL);

  // Add elements to the pipeline
  gst_bin_add_many(GST_BIN(pipeline), src, adaptor1, overlay, videoconvert, videoscale, capsfilter, encoder, muxer, sink, NULL);

  // Link the elements in the pipeline
  if (!gst_element_link_many(src, adaptor1, overlay, videoconvert, videoscale, capsfilter, encoder, muxer, sink, NULL))
  {
    // If linking fails, print an error message and warning
    g_printerr("Failed to link elements\n");
    g_warning("Failed to link elements!");
  }

  // Connect the "draw" signal to the "overlay" element with the "draw_overlay" callback function and the "overlay_state" data
  g_signal_connect(overlay, "draw", G_CALLBACK(draw_overlay), overlay_state);

  // Connect the "caps-changed" signal to the "overlay" element with the "prepare_overlay" callback function and the "overlay_state" data
  g_signal_connect(overlay, "caps-changed", G_CALLBACK(prepare_overlay), overlay_state);

  return pipeline;
}

int main(int argc, char *argv[])
{
  GstElement *pipeline;             // Declare GStreamer pipeline
  GstBus *bus;                      // Declare GStreamer bus
  GMainLoop *loop;                  // Declare GMainLoop
  CairoOverlayState *overlay_state; // Declare Cairo overlay state struct

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
