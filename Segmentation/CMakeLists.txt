cmake_minimum_required(VERSION 2.8)
project( RG_Cam)

#find the required libraries
find_package( OpenCV REQUIRED )
find_package( PythonLibs REQUIRED )

include_directories(inc)
include_directories(${PYTHON_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable( RG_Cam ${SOURCES})
target_link_libraries(RG_Cam ${OpenCV_LIBS} ${PYTHON_LIBRARIES})
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
