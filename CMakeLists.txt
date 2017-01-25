cmake_minimum_required(VERSION 3.6)
project(openvrtest)

include(package.cmake)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

add_executable(openvrtest ${SOURCE_FILES})

## OpenGL / GLU
find_package(OpenGL REQUIRED)

add_definitions(-DGLEW_STATIC)

include_directories(
        .
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/Resources
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIR}
        ${OPENVR_INCLUDE_DIR}
)

message("------------------------" ${OPENVR_LIBRARY})
message("------------------------" ${GLEW_LIBRARY})
message("------------------------" ${OPENGL_LIBRARIES})

target_link_libraries(
        openvrtest
        ${OPENGL_LIBRARIES}
        GLEW::GLEW
        SDL2::SDL2
        ${OPENVR_LIBRARY}
        ${CMAKE_DL_LIBS}
        )

