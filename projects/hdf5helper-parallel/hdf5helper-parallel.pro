#-------------------------------------------------
#
# Project created by QtCreator
#
# hdf5helper-parallel library
#
#-------------------------------------------------

# Old not updated nonfunctional!

TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = hdf5helper-parallel

win32:QMAKE_LFLAGS += /ignore:4099

DEFINES += PARALLEL_HDF5

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../hdf5helper/
DEPENDPATH += $$PWD
DEPENDPATH += $$PWD/../hdf5helper/

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# HDF5 library
include($$PWD/../hdf5-parallel.pri)

SOURCES += \
    $$PWD/../hdf5helper/HDF5File.cpp \
    $$PWD/../hdf5helper/HDF5Vector.cpp \
    $$PWD/../hdf5helper/HDF5Vector3D.cpp \
    $$PWD/../hdf5helper/HDF5Vector4D.cpp \
    $$PWD/../hdf5helper/HDF5Object.cpp \
    $$PWD/../hdf5helper/HDF5Dataset.cpp \
    $$PWD/../hdf5helper/HDF5Group.cpp \
    $$PWD/../hdf5helper/HDF5Attribute.cpp \

HEADERS += \
    hdf5helper-parallel.h \
    $$PWD/../hdf5helper/HDF5File.h \
    $$PWD/../hdf5helper/HDF5Vector.h \
    $$PWD/../hdf5helper/HDF5Vector3D.h \
    $$PWD/../hdf5helper/HDF5Vector4D.h \
    $$PWD/../hdf5helper/HDF5Object.h \
    $$PWD/../hdf5helper/HDF5Dataset.h \
    $$PWD/../hdf5helper/HDF5Group.h \
    $$PWD/../hdf5helper/HDF5Attribute.h \