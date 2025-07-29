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

function(download_dependency_mod REPO_NAME DEPENDENCY_ID)
    set(DEPS_DIR "${CMAKE_BINARY_DIR}/geode-deps/${DEPENDENCY_ID}")
    file(MAKE_DIRECTORY "${DEPS_DIR}")
    set(DOWNLOAD_URL "https://github.com/${REPO_NAME}/releases/latest/download/${DEPENDENCY_ID}.geode")
    set(GEODE_FILE "${DEPS_DIR}/${DEPENDENCY_ID}.geode")
    message(STATUS "Downloading ${DEPENDENCY_ID} from: ${DOWNLOAD_URL}")
    file(DOWNLOAD "${DOWNLOAD_URL}" "${GEODE_FILE}" STATUS DOWNLOAD_STATUS)
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0)
        message(FATAL_ERROR "Failed to download ${DEPENDENCY_ID}: ${DOWNLOAD_STATUS}")
    endif()
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xf "${GEODE_FILE}"
        WORKING_DIRECTORY "${DEPS_DIR}"
        RESULT_VARIABLE EXTRACT_RESULT
    )
    if(NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract ${DEPENDENCY_ID}.geode")
    endif()
    file(REMOVE "${GEODE_FILE}")
    file(WRITE "${DEPS_DIR}/geode-dep-options.json" "{ \"required\": true }")
    message(STATUS "Successfully dependency mod: ${DEPENDENCY_ID}")
endfunction()

download_dependency_mod(user95401/geode-game-objects-factory user95401.game-objects-factory)

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME} EXTERNALS user95401.sprite-frames-unity user95401.game-objects-factory)
