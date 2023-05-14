#include "RTSP_manager.h"

// define constructor and destructor
RTSP_manager::RTSP_manager(GstRTSPServer *server, GstRTSPMountPoints *mounts, GstRTSPMediaFactory *factory, string host, string port, string file_path)
{
    this->host = host;
    this->port = port;
    this->file_path = file_path;
    g_print("RTSP_manager constructor called\n");

    /* Create RTSP server */
    server = gst_rtsp_server_new();
    g_object_set(server, "service", this->port, NULL); // Set the server port

    /* Create RTSP media factory */
    mounts = gst_rtsp_server_get_mount_points(server);
    str = g_strdup_printf("( "
                          "filesrc location=\"%s\" ! qtdemux name=d "
                          "d. ! queue ! rtph264pay pt=96 name=pay0 "
                          "d. ! queue ! rtpmp4apay pt=97 name=pay1 "
                          ")",
                          this->file_path); // Define the pipeline as a GStreamer launch command string
    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, str); // Set the launch command for the factory
    g_signal_connect(factory, "media-configure", (GCallback)RTSP_manager::media_configure_cb,
                     factory); // Connect the media-configure callback function
    g_free(str);
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory); // Add the factory to the mount points
    g_object_unref(mounts);

    /* Attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);
}
RTSP_manager::~RTSP_manager()
{
    g_print("RTSP_manager destructor called\n");
}

// media_configure_cb
static void RTSP_manager::media_configure_cb(GstRTSPMediaFactory *factory, GstRTSPMedia *media)
{
    // g_print("media_configure_cb called\n");

    media_configure_cb(GstRTSPMediaFactory * factory, GstRTSPMedia * media)
    {
        /* connect our prepared signal so that we can see when this media is
         * prepared for streaming */
        g_signal_connect(media, "prepared", (GCallback)RTSP_manager::media_prepared_cb, factory);
    }
}

static void RTSP_manager::media_prepared_cb(GstRTSPMedia *media)
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
        g_signal_connect(session, "on-ssrc-active",
                         (GCallback)RTSP_manager::on_ssrc_active, media);

        // Connect to the "on-sender-ssrc-active" signal of the session and call the "on_sender_ssrc_active" function with the media as the user_data parameter
        g_signal_connect(session, "on-sender-ssrc-active",
                         (GCallback)RTSP_manager::on_sender_ssrc_active, media);
    }
}
/* called when a stream has received an RTCP packet from the client */
static void RTSP_manager::on_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media)
{
    // Declare a pointer to a GstStructure
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

static void RTSP_manager::on_sender_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media)
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