#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "RTP_manager.h"
#include <iostream>
#define DEFAULT_RTSP_PORT "5001"
#define DEFAULT_RTP_PORT 5002
#define DEFAULT_RTSP_HOST "127.0.0.1"
#define DEFAULT_RTP_HOST "127.0.0.1"
#define VIDEO_WIDTH 720
#define VIDEO_HEIGHT 480
#define VIDEO_FPS 50
using namespace std;


int main(int argc, char *argv[])
{
  try
  {
    	auto testRTP = new RTP_manager();
	auto gg=testRTP->test_function();
	std::cout << gg;

	g_print("Running...\n");
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  }
  
  //wait until press enter to close the window
  getchar();

  return 0;
}
