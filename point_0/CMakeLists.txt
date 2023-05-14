# cmake_minimum_required(VERSION 3.10)
set(SUB_PROJECT_NAME "point_0")

#finding pkg-config is a helper tool
find_package(PkgConfig REQUIRED)
# #find_package(Cairo_PKG REQUIRED)
# set(Cairo_DEPS)

# if(PKG_CONFIG_FOUND)
#   pkg_search_module(Cairo_PKG cairo)
# endif()

# find_library(Cairo_LIBRARY cairo HINTS ${Cairo_PKG_LIBRARY_DIRS})
# set(Cairo cairo)
# #print cairo library path
# message(STATUS "Cairo_LIBRARY: ${Cairo_LIBRARY}")

# if(Cairo_LIBRARY)
#   add_library(${Cairo} SHARED IMPORTED)
#   set_property(TARGET ${Cairo} PROPERTY IMPORTED_LOCATION "${Cairo_LIBRARY}")
#   set_property(TARGET ${Cairo} PROPERTY INTERFACE_COMPILE_OPTIONS "${Cairo_PKG_CFLAGS_OTHER}")

#   set(Cairo_INCLUDE_DIRS)

#   find_path(Cairo_INCLUDE_DIR "cairo.h"
#     HINTS ${Cairo_PKG_INCLUDE_DIRS})

#   if(Cairo_INCLUDE_DIR)
#     file(STRINGS "${Cairo_INCLUDE_DIR}/cairo-version.h" Cairo_VERSION_MAJOR REGEX "^#define CAIRO_VERSION_MAJOR +\\(?([0-9]+)\\)?$")
#     string(REGEX REPLACE "^#define CAIRO_VERSION_MAJOR \\(?([0-9]+)\\)?$" "\\1" Cairo_VERSION_MAJOR "${Cairo_VERSION_MAJOR}")
#     file(STRINGS "${Cairo_INCLUDE_DIR}/cairo-version.h" Cairo_VERSION_MINOR REGEX "^#define CAIRO_VERSION_MINOR +\\(?([0-9]+)\\)?$")
#     string(REGEX REPLACE "^#define CAIRO_VERSION_MINOR \\(?([0-9]+)\\)?$" "\\1" Cairo_VERSION_MINOR "${Cairo_VERSION_MINOR}")
#     file(STRINGS "${Cairo_INCLUDE_DIR}/cairo-version.h" Cairo_VERSION_MICRO REGEX "^#define CAIRO_VERSION_MICRO +\\(?([0-9]+)\\)?$")
#     string(REGEX REPLACE "^#define CAIRO_VERSION_MICRO \\(?([0-9]+)\\)?$" "\\1" Cairo_VERSION_MICRO "${Cairo_VERSION_MICRO}")
#     set(Cairo_VERSION "${Cairo_VERSION_MAJOR}.${Cairo_VERSION_MINOR}.${Cairo_VERSION_MICRO}")
#     unset(Cairo_VERSION_MAJOR)
#     unset(Cairo_VERSION_MINOR)
#     unset(Cairo_VERSION_MICRO)

#     list(APPEND Cairo_INCLUDE_DIRS ${Cairo_INCLUDE_DIR})
#     set_property(TARGET ${Cairo} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${Cairo_INCLUDE_DIR}")
#   endif()
# endif()

# set(Cairo_DEPS_FOUND_VARS)
# foreach(cairo_dep ${Cairo_DEPS})
#   find_package(${cairo_dep})

#   list(APPEND Cairo_DEPS_FOUND_VARS "${cairo_dep}_FOUND")
#   list(APPEND Cairo_INCLUDE_DIRS ${${cairo_dep}_INCLUDE_DIRS})

#   set_property (TARGET ${Cairo} APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${${cairo_dep}}")
# endforeach(cairo_dep)

# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(Cairo
#     REQUIRED_VARS
#       Cairo_LIBRARY
#       Cairo_INCLUDE_DIRS
#       ${Cairo_DEPS_FOUND_VARS}
#     VERSION_VAR
#       Cairo_VERSION)

# unset(Cairo_DEPS_FOUND_VARS)
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
        #add cairo
        # ${Cairo_INCLUDE_DIRS}
)

#linking GStreamer library directory
link_directories(
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
        ${GSTREAMER_BASE_LIBRARY_DIRS}
        ${GSTREAMER_RTSP_LIBRARY_DIRS}
        ${GSTREAMER_RTSP_SERVER_LIBRARY_DIRS}
        #add cairo
        # ${Cairo_LIBRARY_DIRS}
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
        #add cairo
        # ${Cairo}
        )
