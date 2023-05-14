#include <gst/gst.h>
#include <glib-unix.h>

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
  GstElement *videoconvert;
  GstElement *videoscale;
  GstElement *encoder;
  GstElement *capsfilter;
  GstElement *muxer;
  GstElement *sink,*filesink;
  GstElement *tee;
  GstPad *muxer_video_pad;
  GstPad *muxer_file_pad;
  GstPad *tee_file_pad,*queue_file_pad;
  GstPad *tee_video_pad,*queue_video_pad;
  GstPad *filesink_pad;
  GstPad *tcpserversink_pad;
  GstElement *video_queue;
  GstElement *file_queue;
  GstCaps *caps;
  GMainLoop *loop;
  gint ret = -1;

  gst_init(NULL, NULL);

  pipeline = gst_pipeline_new("pipeline");

  src = gst_element_factory_make("autovideosrc", "autovideosrc");
  tee = gst_element_factory_make("tee", "tee");
  videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
  videoscale = gst_element_factory_make("videoscale", "videoscale");
  capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
  caps = gst_caps_from_string("video/x-raw,width=640,height=480");
  g_object_set(capsfilter, "caps", caps, NULL);
  gst_caps_unref(caps);
  encoder = gst_element_factory_make("x264enc", "x264enc");
  muxer = gst_element_factory_make("matroskamux", "matroskamux");
  filesink = gst_element_factory_make("filesink", "filesink");
  g_object_set(filesink, "location", "camera_record.mp4", NULL);
  file_queue = gst_element_factory_make("queue", "file_queue");
  sink = gst_element_factory_make("tcpserversink", "tcpserversink");
  video_queue = gst_element_factory_make("queue", "video_queue");

  g_object_set(sink, "host", HOST, "port", 5002, NULL);

  gst_bin_add_many(GST_BIN(pipeline), src, videoconvert, videoscale, capsfilter, encoder, muxer,tee,video_queue, sink,file_queue,filesink, NULL);
  // gst_element_link_many(src, videoconvert, videoscale, capsfilter, encoder, muxer, tee, NULL);
  if(!gst_element_link_many(src, videoconvert, videoscale, capsfilter, encoder, muxer, tee, NULL)){
        g_warning ("Failed to link elements untile tee!");
    }
    if(!gst_element_link_many(video_queue, sink, NULL)){
        g_warning ("Failed to encoder link element with streamer sink ");
    }
    if(!gst_element_link_many(file_queue, filesink, NULL)){
        g_warning ("Failed to encoder link element with file sink ");
    }
  
  tee_video_pad = gst_element_request_pad_simple(tee, "src_%u");
  g_print("Obtained request pad %s for video branch.\n", gst_pad_get_name (tee_video_pad));
  tee_file_pad = gst_element_request_pad_simple(tee, "src_%u");
  g_print("Obtained request pad %s for file branch.\n", gst_pad_get_name (tee_file_pad));

  queue_video_pad = gst_element_get_static_pad(video_queue, "sink");
  queue_file_pad = gst_element_get_static_pad(file_queue, "sink");

if(gst_pad_link(tee_video_pad, queue_video_pad) != GST_PAD_LINK_OK ||
      gst_pad_link(tee_file_pad, queue_file_pad) != GST_PAD_LINK_OK) {
      g_printerr("Tee could not be linked.\n");
      gst_object_unref(pipeline);
      return -1;
  }

  gst_object_unref(queue_video_pad);
  gst_object_unref(queue_file_pad);

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
