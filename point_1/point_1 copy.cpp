

#include <gst/gst.h>
//#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char* argv[]) {
  GstElement* pipeline, * src, * enc, * pay, * sink;
  GstCaps* caps;
  GstBus* bus;
  GstMessage* msg;
  GMainLoop* loop;
  GstElement* filesrc, * qtdemux, * h264parse, * avdec_h264, * x264enc, * rtph264pay, * udpsink;

  GstStateChangeReturn ret;
  /* Initialize GStreamer */
  gst_init(&argc, &argv);
  loop = g_main_loop_new(NULL, FALSE);

  /* Create elements */
  pipeline = gst_pipeline_new("mypipeline");
  src = gst_element_factory_make("videotestsrc", "mysrc");
  enc = gst_element_factory_make("openh264enc", "myenc");
  pay = gst_element_factory_make("rtph264pay", "mypay");
  //sink = gst_element_factory_make("udpsink", "mysink");

  filesrc = gst_element_factory_make("filesrc", "file-source");
  qtdemux = gst_element_factory_make("qtdemux", "quicktime-demuxer");
  h264parse = gst_element_factory_make("h264parse", "h264-parser");
  avdec_h264 = gst_element_factory_make("avdec_h264", "h264-decoder");
  x264enc = gst_element_factory_make("x264enc", "h264-encoder");
  rtph264pay = gst_element_factory_make("rtph264pay", "rtp-h264-payloader");
  udpsink = gst_element_factory_make("udpsink", "udp-sink");

  /* Set properties */
  g_object_set(G_OBJECT(rtph264pay), "config-interval", 10, NULL);
  g_object_set(G_OBJECT(rtph264pay), "pt", 96, NULL);
  //g_object_set(G_OBJECT(sink), "protocols", GST_RTSP_LOWER_TRANS_TCP, NULL);

  g_object_set(G_OBJECT(filesrc), "location", "../video_test_3.mp4", NULL);

  g_object_set(G_OBJECT(udpsink), "host", "127.0.0.1", NULL);
  g_object_set(G_OBJECT(udpsink), "port", 5000, NULL);

  /* Add elements to pipeline */
  gst_bin_add_many(GST_BIN(pipeline), filesrc, qtdemux, h264parse, avdec_h264, x264enc, rtph264pay, udpsink, NULL);

  if (!gst_element_link(filesrc, qtdemux)) {
      g_printerr("File source and Quicktime demuxer could not be linked.\n");
      return -1;
  }
  //if (!gst_element_link(qtdemux, h264parse)) {
  //    g_printerr("Quicktime demuxer and H.264 parser could not be linked.\n");
  //    return -1;
  //}
  if (!gst_element_link(h264parse, avdec_h264)) {
      g_printerr("H.264 parser and H.264 decoder could not be linked.\n");
      return -1;
  }
  if (!gst_element_link(avdec_h264, x264enc)) {
      g_printerr("H.264 decoder and H.264 encoder could not be linked.\n");
      return -1;
  }
  if (!gst_element_link(x264enc, rtph264pay)) {
      g_printerr("H.264 encoder and RTP H.264 payloader could not be linked.\n");
      return -1;
  }
  if (!gst_element_link(rtph264pay, udpsink)) {
      g_printerr("RTP H.264 payloader and UDP sink could not be linked.\n");
      return -1;
  }
  /* Link elements */
  if (!gst_element_link_many(src, enc, pay, sink, NULL)) {
    g_printerr("Failed to link elements\n");
    return -1;
  }

  /* Start playing */
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_print("Running...\n");
  int messageType = GST_MESSAGE_ERROR | GST_MESSAGE_EOS;

  /* Wait until error or EOS */
  bus = gst_element_get_bus(pipeline);
  msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
    (GstMessageType)messageType);

  /* Free resources */
  if (msg != NULL) {
    gst_message_unref(msg);
  }
  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
  g_main_loop_unref(loop);

  return 0;
}