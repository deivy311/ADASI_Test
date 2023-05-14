#include "RTSP_manager.h"

//define constructor and destructor
RTSP_manager::RTSP_manager(GstRTSPServer *server, GstRTSPMountPoints *mounts, GstRTSPMediaFactory *factory )
{
    g_print("RTSP_manager constructor called\n");
    
    /* Create RTSP server */
    server = gst_rtsp_server_new();
    g_object_set(server, "service", DEFAULT_RTSP_PORT, NULL); // Set the server port

    /* Create RTSP media factory */
    mounts = gst_rtsp_server_get_mount_points(server);
    str = g_strdup_printf("( "
                            "filesrc location=\"%s\" ! qtdemux name=d "
                            "d. ! queue ! rtph264pay pt=96 name=pay0 "
                            "d. ! queue ! rtpmp4apay pt=97 name=pay1 "
                            ")",
                            "../Files/video_test_2.mp4"); // Define the pipeline as a GStreamer launch command string
    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, str); // Set the launch command for the factory
    g_signal_connect(factory, "media-configure", (GCallback)media_configure_cb,
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
