cmake_minimum_required(VERSION 2.6)
project(CHIP8)

add_executable(
chip8

${CMAKE_SOURCE_DIR}/core/chip8.h
${CMAKE_SOURCE_DIR}/core/keymap.h
${CMAKE_SOURCE_DIR}/core/chip8.cpp
${CMAKE_SOURCE_DIR}/core/keymap.cpp

${CMAKE_SOURCE_DIR}/test/chip8test.h
${CMAKE_SOURCE_DIR}/test/chip8test.cpp

${CMAKE_SOURCE_DIR}/main/game.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/core/ ${CMAKE_SOURCE_DIR}/test/ ${CMAKE_SOURCE_DIR}/main/)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})
target_link_libraries(chip8 ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
