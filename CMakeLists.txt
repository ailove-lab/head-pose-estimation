cmake_minimum_required(VERSION 3.10)

set(CACHE USE_SSE4_INSTRUCTIONS ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-O3")

include(~/lib/dlib-19.10/dlib/cmake)
find_package( OpenCV REQUIRED )

add_executable(head_pose_estimation head_pose_estimation.cpp)
target_link_libraries(head_pose_estimation ${OpenCV_LIBS} dlib::dlib)


