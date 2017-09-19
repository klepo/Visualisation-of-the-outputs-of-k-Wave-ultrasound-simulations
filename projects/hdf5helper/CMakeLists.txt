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

message("HDF5_PACKAGE_NAME=${HDF5_PACKAGE_NAME}")
message("HDF5_VERSION_STRING=${HDF5_VERSION_STRING}")
message("HDF5_DIR=${HDF5_DIR}")
message("HDF5_ROOT=${HDF5_ROOT}")
message("HDF5_C_INCLUDE_DIR=${HDF5_C_INCLUDE_DIR}")
message("HDF5_DEFINITIONS=${HDF5_DEFINITIONS}")
message("HDF5_C_COMPILER_EXECUTABLE=${HDF5_C_COMPILER_EXECUTABLE}")
#message("HDF5_CXX_INCLUDE_DIR=${HDF5_CXX_INCLUDE_DIR}")
message("HDF5_INCLUDE_DIR=${HDF5_INCLUDE_DIR}")
message("HDF5_INCLUDE_DIRS=${HDF5_INCLUDE_DIRS}")
message("HDF5_LIBRARIES=${HDF5_LIBRARIES}")
message("HDF5_C_LIBRARIES=${HDF5_C_LIBRARIES}")
message("HDF5_C_STATIC_LIBRARY=${HDF5_C_STATIC_LIBRARY}")
message("HDF5_C_SHARED_LIBRARY=${HDF5_C_SHARED_LIBRARY}")

# HDF5 library
set(HDF5_ROOT ${HDF5_ROOT} CACHE PATH "HDF5 root dir")

#if (NOT HDF5_ROOT)
    find_package(HDF5 NAMES hdf5 COMPONENTS C static)
#endif()

if (NOT HDF5_FOUND)
    find_package(HDF5 COMPONENTS C REQUIRED)
endif()

if (HDF5_FOUND)
    if ((NOT HDF5_C_INCLUDE_DIR) AND HDF5_INCLUDE_DIR)
        set(HDF5_C_INCLUDE_DIR ${HDF5_INCLUDE_DIR})
    endif()
    
    if ((NOT HDF5_C_LIBRARIES) AND HDF5_LIBRARIES)
        set(HDF5_C_LIBRARIES ${HDF5_LIBRARIES})
    endif()

    if (WIN32)
        string(REPLACE "include" "" HDF5_ROOT ${HDF5_C_INCLUDE_DIR})
        #message(STATUS "HDF5_ROOT=" ${HDF5_ROOT})
        if (EXISTS "${HDF5_ROOT}lib/libszip_D.lib")
            set(HDF5_LIBRARIES ${HDF5_LIBRARIES} "debug" "${HDF5_ROOT}lib/libszip_D.lib")
        endif()
        if (EXISTS "${HDF5_ROOT}lib/libszip.lib")
            set(HDF5_LIBRARIES ${HDF5_LIBRARIES} "optimized" "${HDF5_ROOT}lib/libszip.lib")
        endif()
        if (EXISTS "${HDF5_ROOT}lib/libzlib_D.lib")
            set(HDF5_LIBRARIES ${HDF5_LIBRARIES} "debug" "${HDF5_ROOT}lib/libzlib_D.lib")
        endif()
        if (EXISTS "${HDF5_ROOT}lib/libzlib.lib")
            set(HDF5_LIBRARIES ${HDF5_LIBRARIES} "optimized" "${HDF5_ROOT}lib/libzlib.lib")
        endif()
    endif()

    set(HDF5_INCLUDE_DIR ${HDF5_C_INCLUDE_DIR})
    set(HDF5_LIBRARIES ${HDF5_C_LIBRARIES})
endif()

message(" ")

message("HDF5_PACKAGE_NAME=${HDF5_PACKAGE_NAME}")
message("HDF5_VERSION_STRING=${HDF5_VERSION_STRING}")
message("HDF5_DIR=${HDF5_DIR}")
message("HDF5_ROOT=${HDF5_ROOT}")
message("HDF5_C_INCLUDE_DIR=${HDF5_C_INCLUDE_DIR}")
message("HDF5_DEFINITIONS=${HDF5_DEFINITIONS}")
message("HDF5_C_COMPILER_EXECUTABLE=${HDF5_C_COMPILER_EXECUTABLE}")
#message("HDF5_CXX_INCLUDE_DIR=${HDF5_CXX_INCLUDE_DIR}")
message("HDF5_INCLUDE_DIR=${HDF5_INCLUDE_DIR}")
message("HDF5_INCLUDE_DIRS=${HDF5_INCLUDE_DIRS}")
message("HDF5_LIBRARIES=${HDF5_LIBRARIES}")
message("HDF5_C_LIBRARIES=${HDF5_C_LIBRARIES}")
message("HDF5_C_STATIC_LIBRARY=${HDF5_C_STATIC_LIBRARY}")
message("HDF5_C_SHARED_LIBRARY=${HDF5_C_SHARED_LIBRARY}")

# OpenMP library
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    if (UNIX)
        set(OpenMP_LINK_FLAG ${OpenMP_CXX_FLAGS})
    endif()
endif()
message(STATUS "PROJECT_NAME=" "${PROJECT_NAME}")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${HDF5_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OpenMP_LINK_FLAG} ${HDF5_LIBRARIES})
