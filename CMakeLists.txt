cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(main)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    # Add any extra C++ source files here
)
include_directories(src)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

file(DOWNLOAD "https://github.com/user95401/geode-download-dependency-cmake/blob/main/.cmake?raw=true" 
    ${CMAKE_CURRENT_BINARY_DIR}/geode-download-dependency.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/geode-download-dependency.cmake)
download_dependency_mod(user95401/geode-game-objects-factory user95401.game-objects-factory)

setup_geode_mod(${PROJECT_NAME} EXTERNALS user95401.sprite-frames-unity user95401.game-objects-factory)
