cmake_minimum_required(VERSION 3.6)
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

message("------------------------" ${OPENVR_LIBRARY})
message("------------------------" ${GLEW_LIBRARY})
message("------------------------" ${OPENGL_LIBRARIES})
message("------------------------ assimp: " ${assimp_INCLUDE_DIRS})

target_link_libraries(
        openvrtest
        ${OPENGL_LIBRARIES}
		${ASSIMP_LIBRARIES}
        GLEW::GLEW
        SDL2::SDL2
        ${OPENVR_LIBRARY}
        ${CMAKE_DL_LIBS}
        )
