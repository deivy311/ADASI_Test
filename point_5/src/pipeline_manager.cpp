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
  /* Define variables */
  GstElement *cairo_overlay;
  GstElement *adaptor1, *adaptor2;
  GstElement *pipeline, *src, *overlay, *capsfilter, *videoconvert, *pay, *sink, *filesink;
  GstElement *tee;
  GstPad *tee_file_pad, *queue_file_pad;
  GstPad *tee_video_pad, *queue_video_pad;
  GstPad *filesink_pad;
  GstPad *tcpserversink_pad;
  GstElement *video_queue;
  GstElement *file_queue;
  GstRTSPMediaFactory *factory;
  gchar *str;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstElement *videoscale;
  GstElement *encoder;
  GstElement *muxer;
  GstCaps *caps;
  GMainLoop *loop;

  /* Create pipeline and elements */
  pipeline = gst_pipeline_new("mypipeline");
  src = gst_element_factory_make(source_type.c_str(), "autovideosrc");     // Creates element for video capture from a device
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
  filesink = gst_element_factory_make("filesink", "filesink");
  g_object_set(filesink, "location", "camera_record.mp4", NULL);
  file_queue = gst_element_factory_make("queue", "file_queue");
  sink = gst_element_factory_make("tcpserversink", "tcpserversink"); // Sends video data to the client over TCP
  video_queue = gst_element_factory_make("queue", "video_queue");

  
  RTSP_manager *rtsp_server_manager = new RTSP_manager();
  overlay_manager *local_overlay_manager = new overlay_manager();
  // RTSP_manager* rtsp_server_manager = new RTSP_manager(server, mounts, factory,DEFAULT_RTSP_HOST,DEFAULT_RTSP_PORT,DEFAULT_FILE_PATH);

  /* Create RTSP server */
  server = gst_rtsp_server_new();
  g_object_set(server, "service", RTSP_port.c_str(), NULL); // Set the server port

  /* Create RTSP media factory */
  mounts = gst_rtsp_server_get_mount_points(server);
  str = g_strdup_printf("( "
                        "filesrc location=\"%s\" ! qtdemux name=d "
                        "d. ! queue ! rtph264pay pt=96 name=pay0 "
                        "d. ! queue ! rtpmp4apay pt=97 name=pay1 "
                        ")",
                        RTSP_file_path.c_str()); // Define the pipeline as a GStreamer launch command string
  factory = gst_rtsp_media_factory_new();
  gst_rtsp_media_factory_set_launch(factory, str); // Set the launch command for the factory
  g_signal_connect(factory, "media-configure", (GCallback)rtsp_server_manager->media_configure_cb,
                   factory); // Connect the media-configure callback function
  g_free(str);
  gst_rtsp_mount_points_add_factory(mounts, "/test", factory); // Add the factory to the mount points
  g_object_unref(mounts);

  /* Attach the server to the default maincontext */
  gst_rtsp_server_attach(server, NULL);
  /* Set TCP server sink properties */
  g_object_set(G_OBJECT(sink), "host", host.c_str(), NULL); // Set the host IP

  // Set the "port" property of the "sink" element to 5002
  g_object_set(G_OBJECT(sink), "port", port, NULL);

  // Add elements to the pipeline
  gst_bin_add_many(GST_BIN(pipeline), src, videoconvert, videoscale, capsfilter, encoder, muxer, tee, video_queue, sink, NULL);//file_queue, filesink, NULL);
  // gst_element_link_many(src, videoconvert, videoscale, capsfilter, encoder, muxer, tee, NULL);
  if (!gst_element_link_many(src, videoconvert, videoscale, capsfilter, encoder, muxer, tee, NULL))
  {
    g_warning("Failed to link elements untile tee!");
  }
  if (!gst_element_link_many(video_queue, sink, NULL))
  {
    g_warning("Failed to encoder link element with streamer sink ");
  }
  // if (!gst_element_link_many(file_queue, filesink, NULL))
  // {
  //   g_warning("Failed to encoder link element with file sink ");
  // }


  // Connect the "draw" signal to the "overlay" element with the "draw_overlay" callback function and the "overlay_state" data
  g_signal_connect(overlay, "draw", G_CALLBACK(local_overlay_manager->draw_overlay), overlay_state);

  // Connect the "caps-changed" signal to the "overlay" element with the "prepare_overlay" callback function and the "overlay_state" data
  g_signal_connect(overlay, "caps-changed", G_CALLBACK(local_overlay_manager->prepare_overlay), overlay_state);

  // tee_video_pad = gst_element_request_pad_simple(tee, "src_%u");
  // g_print("Obtained request pad %s for video branch.\n", gst_pad_get_name(tee_video_pad));
  // tee_file_pad = gst_element_request_pad_simple(tee, "src_%u");
  // g_print("Obtained request pad %s for file branch.\n", gst_pad_get_name(tee_file_pad));

  // queue_video_pad = gst_element_get_static_pad(video_queue, "sink");
  // queue_file_pad = gst_element_get_static_pad(file_queue, "sink");

  // if (gst_pad_link(tee_video_pad, queue_video_pad) != GST_PAD_LINK_OK ||
  //     gst_pad_link(tee_file_pad, queue_file_pad) != GST_PAD_LINK_OK)
  // {
  //   g_printerr("Tee could not be linked.\n");
  //   g_warning("Tee could not be linked.\n");
  //   gst_object_unref(pipeline);
  //   // return -1;
  // }

  // gst_object_unref(queue_video_pad);
  // gst_object_unref(queue_file_pad);

  return pipeline;
}