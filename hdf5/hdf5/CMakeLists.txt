cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(test)

find_package(HDF5 REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${HDF5_INCLUDE_DIRS})
include_directories(${OPENCV_INCLUDE_DIRS})

link_directories(${HDF5_LIBRARY_DIRS})
link_directories(${OPENCV_LIBRARY_DIRS})

add_subdirectory (hdf5file)

add_executable (hdf5test main.cpp)

target_link_libraries (hdf5test hdf5file)