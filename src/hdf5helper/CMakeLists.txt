#-------------------------------------------------
#
# hdf5helper library
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.9)

project(hdf5helper CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
endif()

file(GLOB hdf5helper_SRCS "*.h" "*.cpp")

add_library(${PROJECT_NAME} STATIC ${hdf5helper_SRCS})

# HDF5 library
find_package(HDF5 COMPONENTS C NO_MODULE REQUIRED static)

# OpenMP library
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${HDF5_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${HDF5_C_STATIC_LIBRARY})
