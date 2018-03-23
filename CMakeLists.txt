cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CACHE USE_SSE4_INSTRUCTIONS ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-O3")

include(~/lib/dlib-19.10/dlib/cmake)
find_package( OpenCV REQUIRED )

add_executable(head_pose_estimation head_pose_estimation.cpp)
target_link_libraries(head_pose_estimation ${OpenCV_LIBS} dlib::dlib zmq)

add_executable(zmq_client zmq_client.cpp)
target_link_libraries(zmq_client  zmq)

add_executable(gl_model gl_model.cpp)
target_link_libraries(gl_model ${OpenCV_LIBS} X11 Xi Xmu glut GL GLU m)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
add_executable(glfw_client glad.c glfw_client.c)
target_link_libraries(glfw_client m ${GLFW_LIBRARIES} zmq)


