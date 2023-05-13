#include <gst/gst.h>

int main(int argc, char *argv[]) {
  GstElement *pipeline, *filesrc, *decodebin, *textoverlay, *videoscale, *videoconvert, *videosink;
  GstBus *bus;
  GstMessage *msg;
  GMainLoop *loop;
  gboolean link_ok;

  /* Initialize GStreamer */
  gst_init(&argc, &argv);
  loop = g_main_loop_new(NULL, FALSE);

  /* Create pipeline elements */
  pipeline = gst_pipeline_new("overlay-pipeline");
  filesrc = gst_element_factory_make("videotestsrc", "file-source");
  decodebin = gst_element_factory_make("decodebin", "decoder");
  textoverlay = gst_element_factory_make("textoverlay", "text-overlay");
  videoscale = gst_element_factory_make("videoscale", "video-scale");
  videoconvert = gst_element_factory_make("videoconvert", "video-convert");
  videosink = gst_element_factory_make("autovideosink", "video-sink");

  if (!pipeline || !filesrc || !decodebin || !textoverlay || !videoscale || !videoconvert || !videosink) {
    g_printerr("Not all elements could be created.\n");
    return -1;
  }

  /* Set up pipeline */
  // g_object_set(G_OBJECT(filesrc), "location", "/path/to/video/file.mp4", NULL);
  g_object_set(G_OBJECT(textoverlay), "text", "Hello, world!", NULL);

  /* Add elements to pipeline */
  gst_bin_add_many(GST_BIN(pipeline), filesrc, decodebin, textoverlay, videoscale, videoconvert, videosink, NULL);

  /* Link elements */
  link_ok = gst_element_link(filesrc, decodebin);
  link_ok &= gst_element_link(decodebin, textoverlay);
  link_ok &= gst_element_link_filtered(textoverlay, videoscale, NULL);
  link_ok &= gst_element_link_filtered(videoscale, videoconvert, NULL);
  link_ok &= gst_element_link(videoconvert, videosink);
  if (!link_ok) {
    g_printerr("Elements could not be linked.\n");
    gst_object_unref(pipeline);
    return -1;
  }

  /* Start playing */
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  /* Wait until error or EOS */
  bus = gst_element_get_bus(pipeline);
  msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  /* Free resources */
  if (msg != NULL)
    gst_message_unref(msg);
  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);

  return 0;
}
