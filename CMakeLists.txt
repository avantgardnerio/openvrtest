cmake_minimum_required(VERSION 3.6)
message("-------" ${CMAKE_VS_PLATFORM_TOOLSET})
project(openvrtest)

include(package.cmake)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES 
	main.cpp 
	geom/Matrices.cpp 
	Controller.cpp 
	GlContext.cpp 
	SdlContext.cpp 
	SdlTargetWindow.cpp 
	Square.cpp 
	VrInput.cpp
	AssImpObj.cpp
	NavigateCommand.cpp
	)

add_executable(openvrtest ${SOURCE_FILES})

find_package(assimp REQUIRED)

## OpenGL / GLU
find_package(OpenGL REQUIRED)

#add_definitions(-DGLEW_STATIC)

include_directories(
        .
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/Resources
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIR}
        ${OPENVR_INCLUDE_DIR}
		${assimp_INCLUDE_DIRS}
)

target_link_libraries(
        openvrtest
        ${OPENGL_LIBRARIES}
		${ASSIMP_LIBRARIES}
        GLEW::GLEW
        SDL2::SDL2
        ${OPENVR_LIBRARY}
        ${CMAKE_DL_LIBS}
        )

message("------------------------INCLUDE_DIRECTORIES")
message("[")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "${dir},")
endforeach()
message("]")

message("------------------------target_link_libraries")
message("[")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY target_link_libraries)
foreach(dir ${dirs})
  message(STATUS "${dir},")
endforeach()
message("]")
