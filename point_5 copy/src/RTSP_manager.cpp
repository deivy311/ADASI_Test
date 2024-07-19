#include "RTSP_manager.h"
#include <opencv2/opencv.hpp>
#include <gst/app/gstappsrc.h>

// Define constructor and destructor
RTSP_manager::RTSP_manager()
{
    g_print("RTSP_manager constructor called\n");
}

RTSP_manager::RTSP_manager(GstRTSPServer*& server, GstRTSPMountPoints*& mounts, GstRTSPMediaFactory*& factory, std::string host, std::string port)
{
    this->host = host;
    this->port = port;
    g_print("RTSP_manager constructor called\n");

    /* Create RTSP server */
    server = gst_rtsp_server_new();
    g_object_set(server, "service", this->port.c_str(), NULL); // Set the server port

    /* Create RTSP media factory */
    mounts = gst_rtsp_server_get_mount_points(server);
    factory = gst_rtsp_media_factory_new();

    gchar *str = g_strdup_printf(
        "appsrc name=source is-live=true block=true format=GST_FORMAT_TIME "
        "caps=video/x-raw,format=BGR,width=640,height=480,framerate=30/1 "
        "! videoconvert ! video/x-raw,format=I420 "
        "! x264enc speed-preset=ultrafast tune=zerolatency "
        "! rtph264pay config-interval=1 name=pay0 pt=96");

    gst_rtsp_media_factory_set_launch(factory, str);
    g_signal_connect(factory, "media-configure", (GCallback)RTSP_manager::media_configure_cb, factory);
    g_free(str);

    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_object_unref(mounts);

    /* Attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);
}

RTSP_manager::~RTSP_manager()
{
    g_print("RTSP_manager destructor called\n");
}

void RTSP_manager::media_configure_cb(GstRTSPMediaFactory *factory, GstRTSPMedia *media)
{
    GstElement *element, *appsrc;
    appsrc = gst_bin_get_by_name(GST_BIN(gst_rtsp_media_get_element(media)), "source");

    if (appsrc) {
        g_signal_connect(appsrc, "need-data", G_CALLBACK(RTSP_manager::on_need_data), nullptr);
    } else {
        g_printerr("Failed to get appsrc element\n");
    }
}

void RTSP_manager::on_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
{
    static guint64 timestamp = 0;
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;
    GstMapInfo map;

    cv::Mat frame(480, 640, CV_8UC3, cv::Scalar(60, 60, 60)); // Dark gray background
    static int frame_number = 0;
    frame_number++;
    cv::putText(frame, std::to_string(frame_number), cv::Point(100, 100),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

    size = frame.total() * frame.elemSize();
    buffer = gst_buffer_new_allocate(NULL, size, NULL);

    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, frame.data, size);
    gst_buffer_unmap(buffer, &map);

    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);
    timestamp += GST_BUFFER_DURATION(buffer);

    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);

    if (ret != GST_FLOW_OK) {
        g_printerr("Error pushing buffer: %d\n", ret);
    }
}

void RTSP_manager::media_prepared_cb(GstRTSPMedia *media)
{
    guint i, n_streams;

    // Get the number of streams in the media
    n_streams = gst_rtsp_media_n_streams(media);

    // Log the media and number of streams
    GST_INFO("media %p is prepared and has %u streams", media, n_streams);

    // Iterate through each stream in the media
    for (i = 0; i < n_streams; i++)
    {
        GstRTSPStream *stream;
        GObject *session;

        // Get the stream and skip if it is NULL
        stream = gst_rtsp_media_get_stream(media, i);
        if (stream == NULL)
            continue;

        // Get the session associated with the stream
        session = gst_rtsp_stream_get_rtpsession(stream);

        // Log the session and stream number being watched
        GST_INFO("watching session %p on stream %u", session, i);

        // Connect to the "on-ssrc-active" signal of the session and call the "on_ssrc_active" function with the media as the user_data parameter
        g_signal_connect(session, "on-ssrc-active", (GCallback)RTSP_manager::on_ssrc_active, media);

        // Connect to the "on-sender-ssrc-active" signal of the session and call the "on_sender_ssrc_active" function with the media as the user_data parameter
        g_signal_connect(session, "on-sender-ssrc-active", (GCallback)RTSP_manager::on_sender_ssrc_active, media);
    }
}

/* called when a stream has received an RTCP packet from the client */
void RTSP_manager::on_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media)
{
    GstStructure *stats;

    // Print information about the active source and session
    GST_INFO("source %p in session %p is active", source, session);

    // Get the "stats" property from the source object and assign it to stats
    g_object_get(source, "stats", &stats, NULL);
    if (stats)
    {
        gchar *sstr;

        // Convert the stats structure to a string and print it
        sstr = gst_structure_to_string(stats);
        g_print("structure: %s\n", sstr);

        // Free the string and the stats structure
        g_free(sstr);
        gst_structure_free(stats);
    }
}

void RTSP_manager::on_sender_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media)
{
    GstStructure *stats;

    // Log that the sender's SSRC is active
    GST_INFO("source %p in session %p is active", source, session);

    // Get the stats for the source object
    g_object_get(source, "stats", &stats, NULL);
    if (stats)
    {
        gchar *sstr;

        // Convert the stats structure to a string
        sstr = gst_structure_to_string(stats);

        // Print the sender's stats along with its structure
        g_print("Sender stats:\nstructure: %s\n", sstr);
        g_free(sstr);

        // Free the stats structure
        gst_structure_free(stats);
    }
}
