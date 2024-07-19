#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <opencv2/opencv.hpp>

typedef struct
{
  gboolean white;
  GstClockTime timestamp;
  guint counter;
} MyContext;

static void
need_data(GstElement *appsrc, guint unused, MyContext *ctx)
{
  GstBuffer *buffer;
  GstFlowReturn ret;
  guint size;
  GstMapInfo map;

  // Create an OpenCV image
  cv::Mat image(288, 384, CV_8UC3, cv::Scalar(255, 255, 255));

  // Draw a counter on the image
  std::string text = std::to_string(ctx->counter);
  int fontFace = cv::FONT_HERSHEY_SIMPLEX;
  double fontScale = 5.0; // Increase the font size
  int thickness = 3;
  int baseline = 0;

  // Get the text size and calculate the center position
  cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
  cv::Point textOrg((image.cols - textSize.width) / 2, (image.rows + textSize.height) / 2);

  // Put the text on the image
  cv::putText(image, text, textOrg, fontFace, fontScale, cv::Scalar(0, 0, 255), thickness);

  // Increment the counter
  ctx->counter++;

  // Convert the OpenCV image to a GStreamer buffer
  size = image.total() * image.elemSize();
  buffer = gst_buffer_new_allocate(NULL, size, NULL);
  gst_buffer_map(buffer, &map, GST_MAP_WRITE);
  memcpy(map.data, image.data, size);
  gst_buffer_unmap(buffer, &map);

  GST_BUFFER_PTS(buffer) = ctx->timestamp;
  GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 100); // Faster increments
  ctx->timestamp += GST_BUFFER_DURATION(buffer);

  g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
  gst_buffer_unref(buffer);
}

static void
media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data)
{
  GstElement *element, *appsrc;
  MyContext *ctx;

  element = gst_rtsp_media_get_element(media);

  appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");

  gst_util_set_object_arg(G_OBJECT(appsrc), "format", "time");
  g_object_set(G_OBJECT(appsrc), "caps",
               gst_caps_new_simple("video/x-raw",
                                   "format", G_TYPE_STRING, "BGR",
                                   "width", G_TYPE_INT, 384,
                                   "height", G_TYPE_INT, 288,
                                   "framerate", GST_TYPE_FRACTION, 10, 1, NULL),// verify the framerate
               NULL);

  ctx = g_new0(MyContext, 1);
  ctx->white = FALSE;
  ctx->timestamp = 0;
  ctx->counter = 0;

  g_object_set_data_full(G_OBJECT(media), "my-extra-data", ctx, (GDestroyNotify)g_free);

  g_signal_connect(appsrc, "need-data", (GCallback)need_data, ctx);
  gst_object_unref(appsrc);
  gst_object_unref(element);
}

int main(int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;

  gst_init(&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  server = gst_rtsp_server_new();
  mounts = gst_rtsp_server_get_mount_points(server);

  factory = gst_rtsp_media_factory_new();
  gst_rtsp_media_factory_set_launch(factory,
                                    "( appsrc name=mysrc ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");

  g_signal_connect(factory, "media-configure", (GCallback)media_configure, NULL);

  gst_rtsp_mount_points_add_factory(mounts, "/test", factory);

  g_object_unref(mounts);

  gst_rtsp_server_attach(server, NULL);

  g_print("stream ready at rtsp://127.0.0.1:8554/test\n");
  g_main_loop_run(loop);

  return 0;
}
