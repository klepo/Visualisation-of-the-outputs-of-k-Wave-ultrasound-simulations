#-------------------------------------------------
#
# Main project with subdirs
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)

project(k-wave-h5-projects)

set(DIRS
    k-wave-h5-helper
    k-wave-h5-processing
    h5-rename
    )

option(BUILD_K-WAVE-H5-VISUALIZER "Build the k-wave-h5-visualizer" OFF)

if (BUILD_K-WAVE-H5-VISUALIZER)
    set(DIRS ${DIRS} k-wave-h5-visualizer)
endif()

#message(STATUS "${DIRS}")

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

subdirs(${DIRS})

#include_directories(${DIRS})
