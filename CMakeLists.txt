cmake_minimum_required(VERSION 3.10)

project(ADASITest)

set(CMAKE_CXX_STANDARD 17) #setting C++ 14 standard

# #finding pkg-config is a helper tool
# find_package(PkgConfig REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
file(COPY "Files/" DESTINATION "${CMAKE_BINARY_DIR}/Files")
file(COPY "Files/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Files")

add_subdirectory(point_0)
add_subdirectory(point_1)
add_subdirectory(point_2_3)
add_subdirectory(point_4)
# add_subdirectory(point_5)
#add_subdirectory(point_0)
# add_subdirectory(vulkan_projects)

