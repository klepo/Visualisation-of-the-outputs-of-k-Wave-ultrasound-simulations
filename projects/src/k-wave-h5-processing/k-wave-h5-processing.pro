#-------------------------------------------------
#
# Project created by QtCreator
#
# k-wave-h5-processing console application
#
#-------------------------------------------------

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = k-wave-h5-processing

win32:QMAKE_LFLAGS += /ignore:4099

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# hdf5file library
include($$PWD/../hdf5helper/hdf5helper.pri)

# HDF5 library
include($$PWD/../hdf5.pri)

# OpenCV library
include($$PWD/../opencv.pri)

SOURCES += k-wave-h5-processing.cpp
