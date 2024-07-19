#pragma once
#include <string>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

using namespace std;

class RTSP_manager
{
public:
    // Define constructors and destructor
    RTSP_manager();
    RTSP_manager(GstRTSPServer*& server, GstRTSPMountPoints*& mounts, GstRTSPMediaFactory*& factory, string host, string port);
    RTSP_manager(GstRTSPServer*& server, GstRTSPMountPoints*& mounts, GstRTSPMediaFactory*& factory, string host, string port, char* file_path);
    ~RTSP_manager();

public:
    string host;
    string port;
    char* file_path;
    static void media_configure_cb(GstRTSPMediaFactory *factory, GstRTSPMedia *media);
    static void media_prepared_cb(GstRTSPMedia *media);
    static void on_sender_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media);
    static void on_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media);
    static void on_need_data(GstElement* appsrc, guint unused_size, gpointer user_data);
};
