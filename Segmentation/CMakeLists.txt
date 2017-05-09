cmake_minimum_required(VERSION 2.8)
project( RG_Cam)

#Find required libraries
find_package( OpenCV REQUIRED )
find_package( OpenGL REQUIRED )

#add additional libraries from local folder
add_subdirectory(external)

#Add our personal include folder	
include_directories(inc)

#Add directories from external
include_directories(
	external/glfw-3.1.2/include/GLFW/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	external/assimp-3.0.1270/include/
)

#Group them to a single alias
set(GL_LIBS ${OPENGL_LIBRARY} glfw GLEW_1130)

#Definitions for the OpenGL compiling
add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable( RG_Cam ${SOURCES})
target_link_libraries(RG_Cam ${OpenCV_LIBS} GL_LIB GL_LIBS )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
