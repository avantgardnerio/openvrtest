cmake_minimum_required(VERSION 3.6)
project(openvrtest)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
link_directories("C:/Users/Brent/Downloads/openvr-master/lib/win64")
include_directories("C:/Users/Brent/Downloads/openvr-master/headers")

link_libraries(openvr_api)

add_executable(openvrtest ${SOURCE_FILES})