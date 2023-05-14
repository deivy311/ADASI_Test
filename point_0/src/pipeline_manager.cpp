#include "pipeline_manager.h"

// define constructor and destructor
pipeline_manager::pipeline_manager()
{
    g_print("pipeline_manager constructor called\n");
}
pipeline_manager::~pipeline_manager()
{
    g_print("pipeline_manager destructor called\n");
}

gboolean pipeline_manager::on_message(GstBus *bus, GstMessage *message, gpointer user_data)
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

GstElement *pipeline_manager::setup_gst_pipeline(CairoOverlayState *overlay_state)
{
  /* Define variables */
  GstElement *cairo_overlay;
  GstElement *adaptor1, *adaptor2;
  GstElement *pipeline, *src, *overlay, *capsfilter, *videoconvert, *pay, *sink;
  GstRTSPMediaFactory *factory;
  gchar *str;
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
  overlay_manager* local_overlay_manager = new overlay_manager();
  // RTSP_manager* rtsp_server_manager = new RTSP_manager(server, mounts, factory,DEFAULT_RTSP_HOST,DEFAULT_RTSP_PORT,DEFAULT_FILE_PATH);
  
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
  g_signal_connect(overlay, "draw", G_CALLBACK(local_overlay_manager->draw_overlay), overlay_state);

  // Connect the "caps-changed" signal to the "overlay" element with the "prepare_overlay" callback function and the "overlay_state" data
  g_signal_connect(overlay, "caps-changed", G_CALLBACK(local_overlay_manager->prepare_overlay), overlay_state);

  return pipeline;
}