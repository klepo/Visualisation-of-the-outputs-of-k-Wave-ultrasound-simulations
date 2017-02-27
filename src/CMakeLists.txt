#-------------------------------------------------
#
# Main project with subdirs
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.6)

project(k-wave-h5-projects)

set(dirs 
    hdf5helper 
    k-wave-h5-processing 
    #k-wave-h5-visualizer
    )

subdirs(${dirs})

include_directories(${DIRS})