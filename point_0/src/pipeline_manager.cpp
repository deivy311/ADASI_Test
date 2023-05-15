#include "pipeline_manager.h"

/* Define a variable to hold the start time */
std::chrono::high_resolution_clock::time_point start_time;

// define constructor and destructor
pipeline_manager::pipeline_manager()
{
    g_print("pipeline_manager constructor called\n");
}
pipeline_manager::pipeline_manager(int argc, char* argv[])
{
    g_print("pipeline_manager constructor called\n");
    for (int i = 1; i < argc; i++) {
      std::string parameter_type=argv[i]; 
      if(parameter_type=="-s")
      {
          source_type=argv[i+1];
      }
      if(parameter_type=="-h")
      {
          host=argv[i+1];
      }
      if(parameter_type=="-p")
      {
          port=atoi(argv[i+1]);
      }
    }
}

pipeline_manager::~pipeline_manager()
{
    g_print("pipeline_manager destructor called\n");
}

// define your callback function
gboolean pipeline_manager::my_callback_loop(GstBus* bus, GstMessage* message, gpointer user_data) {
    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ELEMENT) {
        g_print("Got");
        const GstStructure* structure = gst_message_get_structure(message);
        if (gst_structure_has_name(structure, "preroll-frame")) {
            // extract the buffer from the message
            GstBuffer* buffer;
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
  //print typ of message
  //g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME(message));
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
      const GstStructure* s = gst_message_get_structure(message);
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
          g_print("Delay: %lld ms\n", delay_ms);
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

GstElement *pipeline_manager::setup_gst_pipeline(std::string source_type, std::string host, int port)
{
  /* Define variables */
  GstElement *adaptor1, *adaptor2;
  GstElement *pipeline, *src, *overlay, *capsfilter, *videoconvert, *pay, *sink;
  gchar *str;
  GstElement *videoscale;
  GstElement *encoder;
  GstElement *muxer;
  GstElement* queue;
  GstCaps *caps;
  GMainLoop *loop;

  /* Create pipeline and elements */
  pipeline = gst_pipeline_new("mypipeline");
  src = gst_element_factory_make("videotestsrc", "autovideosrc");          // Creates element for video capture from a device
  adaptor1 = gst_element_factory_make("videoconvert", "adaptor1");         // Converts between various video formats
  videoconvert = gst_element_factory_make("videoconvert", "videoconvert"); // Converts between various video formats
  videoscale = gst_element_factory_make("videoscale", "videoscale");       // Scales the video frame
  capsfilter = gst_element_factory_make("capsfilter", "capsfilter");       // Limits the capabilities of the pipeline
  caps = gst_caps_from_string("video/x-raw,width=640,height=480");         // Set the resolution of the video
  g_object_set(capsfilter, "caps", caps, NULL);                            // Set the capsfilter to limit capabilities
  gst_caps_unref(caps);
  encoder = gst_element_factory_make("x264enc", "x264enc");          // Compresses video with the x264 codec
  muxer = gst_element_factory_make("matroskamux", "matroskamux");    // Muxes different streams of data into a Matroska file format
  queue = gst_element_factory_make("queue", "queue");
  sink = gst_element_factory_make("tcpserversink", "tcpserversink"); // Sends video data to the client over TCP
  
  /* Set TCP server sink properties */
  g_object_set(G_OBJECT(sink), "host", host, NULL); // Set the host IP

  // Set the "port" property of the "sink" element to 5002
  g_object_set(G_OBJECT(sink), "port", port, NULL);

  // Add elements to the pipeline
  gst_bin_add_many(GST_BIN(pipeline), src, adaptor1, videoconvert, videoscale, capsfilter, encoder, muxer, queue,sink, NULL);

  // Link the elements in the pipeline
  if (!gst_element_link_many(src, adaptor1, videoconvert, videoscale, capsfilter, encoder, muxer, queue, sink, NULL))
  {
    // If linking fails, print an error message and warning
    g_printerr("Failed to link elements\n");
    g_warning("Failed to link elements!");
  }
  return pipeline;
}