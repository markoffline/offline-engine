cmake_minimum_required(VERSION 3.23)
project(offline_engine)

set(CMAKE_CXX_STANDARD 17)

add_executable(offline_engine main.cpp Engine.cpp Engine.h)

target_link_libraries(offline_engine vulkan glfw)
