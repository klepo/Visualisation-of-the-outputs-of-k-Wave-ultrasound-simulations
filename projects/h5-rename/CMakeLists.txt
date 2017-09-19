#-------------------------------------------------
#
# h5-rename console application
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)

project(h5-rename CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SRCS "*.h" "*.cpp")
file(GLOB SRCS2 "../k-wave-h5-processing/paramsdefinition.h" "../k-wave-h5-processing/helper.h" "../k-wave-h5-processing/paramsdefinition.cpp" "../k-wave-h5-processing/helper.cpp")

add_executable(${PROJECT_NAME} ${SRCS} ${SRCS2})

# Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

message(STATUS "PROJECT_NAME=" "${PROJECT_NAME}")

# hdf5helper library
target_link_libraries(${PROJECT_NAME} hdf5helper)
