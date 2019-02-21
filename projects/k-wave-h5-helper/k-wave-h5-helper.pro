#-------------------------------------------------
#
# Project created by QtCreator
#
# k-wave-h5-helper library
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = k-wave-h5-helper

VERSION = 1.1

win32:QMAKE_LFLAGS += /ignore:4099
win32:QMAKE_CXXFLAGS += -wd4068

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# OpenMP library
include($$PWD/../openmp.pri)

# HDF5 library
include($$PWD/../hdf5.pri)

SOURCES += \
    attribute.cpp \
    compresshelper.cpp \
    dataset.cpp \
    file.cpp \
    group.cpp \
    object.cpp \
    vector.cpp \
    vector3d.cpp \
    vector4d.cpp \

HEADERS += \
    attribute.h \
    compresshelper.h \
    dataset.h \
    file.h \
    group.h \
    k-wave-h5-helper.h \
    object.h \
    vector.h \
    vector3d.h \
    vector4d.h \
