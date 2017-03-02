#-------------------------------------------------
#
# k-wave-h5-processing console application
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)

project(k-wave-h5-processing CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB k-wave-h5-processing_SRCS "*.h" "*.cpp")

add_executable(${PROJECT_NAME} ${k-wave-h5-processing_SRCS})

# OpenMP library
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# hdf5helper library
target_link_libraries(${PROJECT_NAME} hdf5helper)
