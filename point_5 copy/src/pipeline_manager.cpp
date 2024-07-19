#include "pipeline_manager.h"

/* Define a variable to hold the start time */
std::chrono::high_resolution_clock::time_point start_time;

// define constructor and destructor
pipeline_manager::pipeline_manager()
{
  g_print("pipeline_manager constructor called\n");
}
pipeline_manager::pipeline_manager(int argc, char *argv[])
{
  g_print("pipeline_manager constructor called\n");
  for (int i = 1; i < argc; i++)
  {
    std::string parameter_type = argv[i];

    if (parameter_type == "-s")
    {
      source_type = argv[i + 1];
    }
    if (parameter_type == "-h")
    {
      host = argv[i + 1];
    }
    if (parameter_type == "-p")
    {
      port = atoi(argv[i + 1]);
    }
    if (parameter_type == "-p_rtsp")
    {
      RTSP_port = argv[i + 1];
    }
    if (parameter_type == "-f")
    {
      RTSP_file_path = argv[i + 1];
    }
  }
  // priting default parameters
  g_print("source_type: %s\n", source_type.c_str());
  g_print("RTP host: %s\n", host.c_str());
  g_print("RTP port: %d\n", port);
  g_print("RTSP port: %s\n", RTSP_port.c_str());
  g_print("RTSP file path: %s\n", RTSP_file_path.c_str());
}

pipeline_manager::~pipeline_manager()
{
  g_print("pipeline_manager destructor called\n");
}

// define your callback function
gboolean pipeline_manager::my_callback_loop(GstBus *bus, GstMessage *message, gpointer user_data)
{
  if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ELEMENT)
  {
    g_print("Got");
    const GstStructure *structure = gst_message_get_structure(message);
    if (gst_structure_has_name(structure, "preroll-frame"))
    {
      // extract the buffer from the message
      GstBuffer *buffer;
      gst_structure_get(structure, "buffer", GST_TYPE_BUFFER, &buffer, NULL);

      // do something with the buffer here
      g_print("Got preroll buffer with timestamp %" GST_TIME_FORMAT "\n", GST_TIME_ARGS(GST_BUFFER_TIMESTAMP(buffer)));

      gst_buffer_unref(buffer);
    }
  }
  return true; // return true to continue receiving messages, or false to stop
}
gboolean pipeline_manager::on_message(GstBus *bus, GstMessage *message, gpointer user_data)
{
  GMainLoop *loop = (GMainLoop *)user_data;
  // print typ of message
  // g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME(message));
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
  case GST_MESSAGE_STATE_CHANGED:
  case GST_MESSAGE_ELEMENT:
  {
    const GstStructure *s = gst_message_get_structure(message);
    if (gst_structure_has_name(s, "GstFPS"))
    {
      /* Report FPS */
      gdouble fps, interval, droppable_fps, average_fps;
      gst_structure_get_double(s, "instantaneous-fps", &fps);
      gst_structure_get_double(s, "interval", &interval);
      gst_structure_get_double(s, "dropped-fps", &droppable_fps);
      gst_structure_get_double(s, "average-fps", &average_fps);
      g_print("FPS: %.2f, Interval: %.2f, Droppable FPS: %.2f, Average FPS: %.2f\n",
              fps, interval, droppable_fps, average_fps);
    }
    else if (gst_structure_has_name(s, "GstBuffer"))
    {
      /* Report delay */
      auto end_time = std::chrono::high_resolution_clock::now();
      auto delay_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
      // g_print("Delay: %lld ms\n", delay_ms);
    }
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

GstElement *pipeline_manager::setup_gst_pipeline(CairoOverlayState *overlay_state, std::string source_type, std::string host, int port, std::string RTSP_port, std::string RTSP_file_path)
{
    std::string rtsp_uri = "rtsp://192.168.20.10:554/live0";

    /* Define variables */
    GstElement *pipeline, *src, *depay, *decodebin, *videoconvert, *pay;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    gchar *launch_string;

    /* Create pipeline and elements */
    pipeline = gst_pipeline_new("mypipeline");
    src = gst_element_factory_make("rtspsrc", "rtsp-src");
    g_object_set(src, "location", rtsp_uri.c_str(), NULL);

    depay = gst_element_factory_make("rtph264depay", "depay");
    decodebin = gst_element_factory_make("decodebin", "decodebin");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    pay = gst_element_factory_make("rtph264pay", "pay0");

    /* Add elements to the pipeline */
    gst_bin_add_many(GST_BIN(pipeline), src, depay, decodebin, videoconvert, pay, NULL);

    /* Link the elements in the pipeline */
    if (!gst_element_link_many(src, depay, decodebin, videoconvert, pay, NULL))
    {
        g_warning("Failed to link elements in the pipeline!");
        return NULL;
    }

    /* Create RTSP server */
    server = gst_rtsp_server_new();
    g_object_set(server, "service", RTSP_port.c_str(), NULL);

    /* Create RTSP media factory */
    mounts = gst_rtsp_server_get_mount_points(server);
    factory = gst_rtsp_media_factory_new();
    launch_string = g_strdup_printf("( rtspsrc location=%s ! rtph264depay ! decodebin ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )", rtsp_uri.c_str());
    gst_rtsp_media_factory_set_launch(factory, launch_string);
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_object_unref(mounts);
    g_free(launch_string);

    /* Attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);

    return pipeline;
}
