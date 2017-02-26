#-------------------------------------------------
#
# k-wave-h5-processing console application
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.6)

project(k-wave-h5-processing CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
endif()

add_executable(k-wave-h5-processing
    helper.cpp
    settings.cpp
    filescontext.cpp
    dtsforpcs.cpp
    paramsdefinition.cpp
    processing.cpp
    main.cpp
    helper.h
    settings.h
    filescontext.h
    dtsforpcs.h
    paramsdefinition.h
    processing.h
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# OpenMP library
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# HDF5 library
find_package(HDF5 COMPONENTS C HL NO_MODULE REQUIRED static)
#FIND_PACKAGE (HDF5 NAMES hdf5)
include_directories(${HDF5_INCLUDE_DIR})

# hdf5helper library
target_link_libraries(k-wave-h5-processing hdf5helper ${HDF5_C_STATIC_LIBRARY})