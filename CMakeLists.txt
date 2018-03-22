cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CACHE USE_SSE4_INSTRUCTIONS ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-O3")

include(~/lib/dlib-19.10/dlib/cmake)
find_package( OpenCV REQUIRED )

add_executable(head_pose_estimation head_pose_estimation.cpp)
target_link_libraries(head_pose_estimation ${OpenCV_LIBS} dlib::dlib)

add_executable(gl_model gl_model.cpp)
target_link_libraries(gl_model ${OpenCV_LIBS} X11 Xi Xmu glut GL GLU m)
