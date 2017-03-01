#-------------------------------------------------
#
# Main project with subdirs
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.9)

project(k-wave-h5-projects)

set(DIRS
    hdf5helper
    k-wave-h5-processing
    k-wave-h5-visualizer
    )

subdirs(${DIRS})

include_directories(${DIRS})
