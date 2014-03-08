cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(hdf5file)

find_package(HDF5 REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${HDF5_INCLUDE_DIRS})
include_directories(${OPENCV_INCLUDE_DIRS})

link_directories(${HDF5_LIBRARY_DIRS})
link_directories(${OPENCV_LIBRARY_DIRS})

#"C:/Program Files/HDF_Group/HDF5/1.8.11/include"
#"C:/Program Files/HDF_Group/HDF5/1.8.11/lib"
#"E:/opencv/build/include"
#"E:/opencv/build/x64/vc11/lib"

add_library(hdf5file hdf5file.cpp hdf5file.h)

target_link_libraries (hdf5file ${HDF5_LIBRARY_DIRS} ${HDF5_LIBRARIES})
target_link_libraries (hdf5file ${OPENCV_LIBRARY_DIRS} ${OPENCV_LIBRARIES})


