#-------------------------------------------------
#
# Project created by QtCreator
#
# hdf5helper library
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = hdf5helper

VERSION = 1.1

win32:QMAKE_LFLAGS += /ignore:4099

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# OpenMP library
include($$PWD/../openmp.pri)

# HDF5 library
include($$PWD/../hdf5.pri)

SOURCES += \
    file.cpp \
    attribute.cpp \
    dataset.cpp \
    group.cpp \
    object.cpp \
    vector.cpp \
    vector3d.cpp \
    vector4d.cpp

HEADERS += \
    hdf5helper.h \
    file.h \
    attribute.h \
    dataset.h \
    group.h \
    object.h \
    vector.h \
    vector3d.h \
    vector4d.h

