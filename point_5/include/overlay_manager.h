#pragma once
#include <string>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <cairo.h>
#include <cairo-gobject.h>
#include <glib.h>
#include <gst/rtsp-server/rtsp-server.h>

using namespace std;
/* Datastructure to share the state we are interested in between
 * prepare and render function. */
typedef struct
{
  gboolean valid;
  GstVideoInfo vinfo;
} CairoOverlayState;
class overlay_manager
{
public:
    //define constructor and destructor
	overlay_manager();
	overlay_manager(GstRTSPServer*& server, GstRTSPMountPoints*& mounts, GstRTSPMediaFactory*& factory , string host ,string port, char* file_path);
	~overlay_manager();
public:

	static void	prepare_overlay(GstElement *overlay, GstCaps *caps, gpointer user_data);
	static void draw_overlay(GstElement *overlay, cairo_t *cr, guint64 timestamp,
                         guint64 duration, gpointer user_data);

};

