#-------------------------------------------------
#
# Main project with subdirs
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8)

project(k-wave-h5-projects)

set(DIRS
    hdf5helper
    k-wave-h5-processing
    )
    
option(BUILD_K-WAVE-H5-VISUALIZER "Build the k-wave-h5-visualizer" ON)

if (BUILD_VISUALIZER)
    set(DIRS ${DIRS} k-wave-h5-visualizer)
endif()

#message(STATUS "${DIRS}")

subdirs(${DIRS})

include_directories(${DIRS})
