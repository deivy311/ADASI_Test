#pragma once
#include <string>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

using namespace std;
class RTSP_manager
{
public:
    //define constructor and destructor
	RTSP_manager();
	RTSP_manager(GstRTSPServer* server, GstRTSPMountPoints* mounts, GstRTSPMediaFactory* factory , string host ,string port, string file_path);
	~RTSP_manager();
private:
	string host;
	string port;
	string file_path;
	static void media_configure_cb(GstRTSPMediaFactory *factory, GstRTSPMedia *media);
	static void media_prepared_cb(GstRTSPMedia *media);
	static void on_sender_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media);
	static void on_ssrc_active(GObject *session, GObject *source, GstRTSPMedia *media);
};

