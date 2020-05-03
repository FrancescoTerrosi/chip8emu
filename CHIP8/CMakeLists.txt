cmake_minimum_required(VERSION 2.6)
project(CHIP8)

add_executable(
chip8

${CMAKE_SOURCE_DIR}/chip8.h
${CMAKE_SOURCE_DIR}/chip8.cpp
${CMAKE_SOURCE_DIR}/chip8test.cpp
${CMAKE_SOURCE_DIR}/game.cpp
)
