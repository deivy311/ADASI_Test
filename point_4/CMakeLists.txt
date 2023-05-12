cmake_minimum_required(VERSION 3.10)

project(point_4)

set(CMAKE_CXX_STANDARD 14) #setting C++ 14 standard

#finding pkg-config is a helper tool
find_package(PkgConfig REQUIRED)

#using pkg-config to getting Gstreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-video-1.0 gstreamer-audio-1.0)
pkg_search_module(GSTREAMER_RTSP_SERVER REQUIRED gstreamer-rtsp-server-1.0)
#including GStreamer header files directory
include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER_BASE_INCLUDE_DIRS}
        ${GSTREAMER_RTSP_INCLUDE_DIRS}
        ${GSTREAMER_RTSP_SERVER_INCLUDE_DIRS}
)

#linking GStreamer library directory
link_directories(
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
        ${GSTREAMER_BASE_LIBRARY_DIRS}
        ${GSTREAMER_RTSP_LIBRARY_DIRS}
        ${GSTREAMER_RTSP_SERVER_LIBRARY_DIRS}

)

#building target executable
add_executable(${PROJECT_NAME} point_4.cpp)

#linking Gstreamer library with target executable
target_link_libraries(${PROJECT_NAME} ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARIES}
        ${GSTREAMER_RTSP_LIBRARIES}
        ${GSTREAMER_RTSP_SERVER_LIBRARIES}
        ${GSTREAMER_RTSP_SERVER_LDFLAGS}
        )
