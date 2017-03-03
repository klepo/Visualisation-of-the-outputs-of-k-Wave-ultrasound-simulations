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

if (WIN32)
    get_target_property(QT_LOCATION Qt5::Widgets LOCATION)
    get_filename_component(QT_DLL_DIR ${QT_LOCATION} PATH)

    message(STATUS ${QT_DLL_DIR})

    if(NOT (EXISTS "${CMAKE_BINARY_DIR}/Release"))
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Release")
    endif()

    if(NOT (EXISTS "${CMAKE_BINARY_DIR}/Release/platforms"))
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Release/platforms")
    endif()

    if(NOT (EXISTS "${CMAKE_BINARY_DIR}/Debug"))
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
    endif()

    if(NOT (EXISTS "${CMAKE_BINARY_DIR}/Debug/platforms"))
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Debug/platforms")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_DLL_DIR}/Qt5Cored.dll"
        "${QT_DLL_DIR}/Qt5Guid.dll"
        "${QT_DLL_DIR}/Qt5Widgetsd.dll"
        "${QT_DLL_DIR}/icudt54.dll"
        "${QT_DLL_DIR}/icuin54.dll"
        "${QT_DLL_DIR}/icuuc54.dll"
        "${CMAKE_BINARY_DIR}/Debug")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_DLL_DIR}/../plugins/platforms/qminimald.dll"
        "${QT_DLL_DIR}/../plugins/platforms/qoffscreend.dll"
        "${QT_DLL_DIR}/../plugins/platforms/qwindowsd.dll"
        "${CMAKE_BINARY_DIR}/Debug/platforms")        

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_DLL_DIR}/Qt5Core.dll"
        "${QT_DLL_DIR}/Qt5Gui.dll"
        "${QT_DLL_DIR}/Qt5Widgets.dll"
        "${QT_DLL_DIR}/icudt54.dll"
        "${QT_DLL_DIR}/icuin54.dll"
        "${QT_DLL_DIR}/icuuc54.dll"
        "${CMAKE_BINARY_DIR}/Release")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_DLL_DIR}/../plugins/platforms/qminimal.dll"
        "${QT_DLL_DIR}/../plugins/platforms/qoffscreen.dll"
        "${QT_DLL_DIR}/../plugins/platforms/qwindows.dll"
        "${CMAKE_BINARY_DIR}/Release/platforms")

endif()