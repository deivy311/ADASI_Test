#pragma once
#include <string>
#include <gst/gst.h>

using namespace std;
class RTSP_manager
{
public:
    //define constructor and destructor
	RTSP_manager();
	RTSP_manager(GstRTSPServer* server, GstRTSPMountPoints* mounts, GstRTSPMediaFactory* factory );
	~RTSP_manager();
	string test_function();
};

