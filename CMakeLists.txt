cmake_minimum_required(VERSION 3.6)
project(openvrtest)

include(package.cmake)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

add_executable(openvrtest ${SOURCE_FILES})

include_directories(
        .
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/Resources
        ${OPENVR_INCLUDE_DIR}
)

message(${OPENVR_LIBRARY})
#message(${CGET_ARCH})

target_link_libraries(
        openvrtest
        ${OPENVR_LIBRARY}
        ${CMAKE_DL_LIBS}
        )

