#-------------------------------------------------
#
# k-wave-h5-processing console application
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8)

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

# hdf5helper library
target_link_libraries(${PROJECT_NAME} hdf5helper)
