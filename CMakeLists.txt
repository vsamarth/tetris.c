cmake_minimum_required(VERSION 3.21)

# Prevent in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# Install CPM.cmake
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

project(
    tetris
    VERSION 0.1.0
    DESCRIPTION "Tetris game"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)

CPMAddPackage(
  NAME raylib
  GITHUB_REPOSITORY raysan5/raylib
  VERSION 5.0
  GIT_TAG 5.0
)

add_executable(tetris src/main.c)

target_link_libraries(tetris raylib)