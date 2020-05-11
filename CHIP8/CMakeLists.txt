cmake_minimum_required(VERSION 2.6)
project(CHIP8)

add_executable(
chip8

${CMAKE_SOURCE_DIR}/chip8.h
${CMAKE_SOURCE_DIR}/chip8test.h
${CMAKE_SOURCE_DIR}/keymap.h
${CMAKE_SOURCE_DIR}/chip8.cpp
${CMAKE_SOURCE_DIR}/chip8test.cpp
${CMAKE_SOURCE_DIR}/keymap.cpp
${CMAKE_SOURCE_DIR}/game.cpp
)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
target_link_libraries(chip8 ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
