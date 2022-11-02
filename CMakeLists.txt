cmake_minimum_required(VERSION 3.23)
project(vk_sample)

set(CMAKE_CXX_STANDARD 17)

add_executable(vk_sample main.cpp Engine.cpp Engine.h)

target_link_libraries(vk_sample glfw)
