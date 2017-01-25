cmake_minimum_required(VERSION 3.6)
project(openvrtest)

include(package.cmake)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

add_executable(openvrtest ${SOURCE_FILES})

## OpenVR API path
find_library(OPENVR_LIBRARIES
        NAMES
        openvr_api
        PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/bin
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        PATH_SUFFIXES
        linux64
        osx32
        win64
        ${WINDOWS_PATH_SUFFIXES}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        )

include_directories(
        .
        ${OPENVR_INCLUDE_DIR}
)