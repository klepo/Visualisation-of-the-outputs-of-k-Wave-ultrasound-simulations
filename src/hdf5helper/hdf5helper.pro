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
#CONFIG += build_all

TARGET = hdf5helper

VERSION = 1.1

win32:QMAKE_LFLAGS += /ignore:4099

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# HDF5 library
include($$PWD/../hdf5.pri)

SOURCES += \
    HDF5File.cpp \
    HDF5Vector.cpp \
    HDF5Vector3D.cpp \
    HDF5Vector4D.cpp \
    HDF5Object.cpp \
    HDF5Dataset.cpp \
    HDF5Group.cpp \
    HDF5Attribute.cpp \


HEADERS += \
    hdf5helper.h \
    HDF5File.h \
    HDF5Vector.h \
    HDF5Vector3D.h \
    HDF5Vector4D.h \
    HDF5Object.h \
    HDF5Dataset.h \
    HDF5Group.h \
    HDF5Attribute.h \

