#-------------------------------------------------
#
# hdf5helper library
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.6)

project(hdf5helper CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
endif()

add_library(hdf5helper STATIC
    HDF5File.cpp
    HDF5Vector.cpp
    HDF5Vector3D.cpp
    HDF5Vector4D.cpp
    HDF5Object.cpp
    HDF5Dataset.cpp
    HDF5Group.cpp
    HDF5Attribute.cpp
    hdf5helper.h
    HDF5File.h
    HDF5Vector.h
    HDF5Vector3D.h
    HDF5Vector4D.h
    HDF5Object.h
    HDF5Dataset.h
    HDF5Group.h
    HDF5Attribute.h
    )

# HDF5 library
find_package(HDF5 COMPONENTS C HL NO_MODULE REQUIRED static)
#find_package (HDF5 NAMES hdf5 COMPONENTS C static)
#FIND_PACKAGE (HDF5 NAMES hdf5)
include_directories(${HDF5_INCLUDE_DIR})

target_include_directories(hdf5helper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

