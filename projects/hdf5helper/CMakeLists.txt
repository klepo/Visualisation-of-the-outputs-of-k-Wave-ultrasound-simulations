#-------------------------------------------------
#
# hdf5helper library
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)

project(hdf5helper CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
endif()

file(GLOB SRCS "*.h" "*.cpp")

add_library(${PROJECT_NAME} STATIC ${SRCS})

# HDF5 library
find_package(HDF5 NAMES hdf5 COMPONENTS C static)
if (NOT HDF5_FOUND)
    #set(HDF5_USE_STATIC_LIBRARIES ON)
    find_package(HDF5 REQUIRED)
endif()

# OpenMP library
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${HDF5_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_FLAGS} ${HDF5_C_STATIC_LIBRARY} ${HDF5_LIBRARIES})
