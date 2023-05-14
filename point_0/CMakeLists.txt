# cmake_minimum_required(VERSION 3.10)
set(SUB_PROJECT_NAME "point_0")

#finding pkg-config is a helper tool
find_package(PkgConfig REQUIRED)

#using pkg-config to getting Gstreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-video-1.0 gstreamer-audio-1.0)
pkg_search_module(GSTREAMER_RTSP_SERVER REQUIRED gstreamer-rtsp-server-1.0)
#including GStreamer header files directory
include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
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

# Add source files
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

#print SOURCE_FILES
message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")

SET(SOURCE_FILES_LOCAL ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(HEADER_FILES_LOCAL ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Add source files

# Add header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
#print CMAKE_CURRENT_SOURCE_DIR
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
#building target executable
# add_executable(${SUB_PROJECT_NAME} point_0.cpp ${SOURCE_FILES_LOCAL} ${HEADER_FILES_LOCAL})
add_executable(${SUB_PROJECT_NAME} point_0.cpp  ${SOURCE_FILES})
# link the exe against the libraries
# target_sources(${SUB_PROJECT_NAME} PRIVATE ${SOURCE_FILES_LOCAL})

target_include_directories(${SUB_PROJECT_NAME}
    PUBLIC
    ${HEADER_FILES_LOCAL}
    ${CMAKE_CURRENT_BINARY_DIR}
)
# #linking Gstreamer library with target executable
target_link_libraries(${SUB_PROJECT_NAME} ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARIES}
        ${GSTREAMER_RTSP_LIBRARIES}
        ${GSTREAMER_RTSP_SERVER_LIBRARIES}
        ${GSTREAMER_RTSP_SERVER_LDFLAGS}
        )
