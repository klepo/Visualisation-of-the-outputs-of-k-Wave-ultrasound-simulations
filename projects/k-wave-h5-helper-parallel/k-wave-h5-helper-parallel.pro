#-------------------------------------------------
#
# Project created by QtCreator
#
# k-wave-h5-helper-parallel library
#
#-------------------------------------------------

# Old not updated nonfunctional!

TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = k-wave-h5-helper-parallel

win32:QMAKE_LFLAGS += /ignore:4099

DEFINES += PARALLEL_HDF5

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../k-wave-h5-helper/
DEPENDPATH += $$PWD
DEPENDPATH += $$PWD/../k-wave-h5-helper/

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# HDF5 library
include($$PWD/../hdf5-parallel.pri)

SOURCES += \
    $$PWD/../k-wave-h5-helper/file.cpp \
    $$PWD/../k-wave-h5-helper/attribute.cpp \
    $$PWD/../k-wave-h5-helper/dataset.cpp \
    $$PWD/../k-wave-h5-helper/group.cpp \
    $$PWD/../k-wave-h5-helper/object.cpp \
    $$PWD/../k-wave-h5-helper/vector.cpp \
    $$PWD/../k-wave-h5-helper/vector3d.cpp \
    $$PWD/../k-wave-h5-helper/vector4d.cpp \

HEADERS += \
    k-wave-h5-helper-parallel.h \
    $$PWD/../k-wave-h5-helper/file.h \
    $$PWD/../k-wave-h5-helper/attribute.h \
    $$PWD/../k-wave-h5-helper/dataset.h \
    $$PWD/../k-wave-h5-helper/group.h \
    $$PWD/../k-wave-h5-helper/object.h \
    $$PWD/../k-wave-h5-helper/vector.h \
    $$PWD/../k-wave-h5-helper/vector3d.h \
    $$PWD/../k-wave-h5-helper/vector4d.h \
