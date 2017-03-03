#-------------------------------------------------
#
# k-wave-h5-visualizer gui application
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)

project(k-wave-h5-visualizer CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)

file(GLOB SRCS "*.h" "*.cpp" "*.qrc" "*.ui" "*.rc")
file(GLOB UIS "*.ui")

qt5_wrap_ui(UIS_WRAPPED ${UIS})

add_executable(${PROJECT_NAME} ${SRCS} ${UIS_WRAPPED})

# hdf5helper library
target_link_libraries(${PROJECT_NAME} hdf5helper Qt5::Widgets)
