cmake_minimum_required(VERSION 3.5)

project(Learn_opengl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(PROJECT_SOURCE_DIR ${PROJECT_BINARY_DIR}/src)
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

include_directories( ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/common)
add_library(common common/shader.cpp)


add_executable(Learn_opengl main.cpp)
#add_executable(Learn_opengl ${PROJECT_SOURCE_DIR}/main.cpp)
target_link_libraries(Learn_opengl ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} glfw common)
