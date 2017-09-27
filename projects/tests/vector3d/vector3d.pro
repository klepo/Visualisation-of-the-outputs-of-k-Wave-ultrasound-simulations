#-------------------------------------------------
#
# Project created by QtCreator 2016-06-29T14:30:13
#
#-------------------------------------------------

QT += testlib
QT -= gui
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = vector3d

win32:QMAKE_LFLAGS += /ignore:4099

SOURCES += vector3d.cpp

# HDF5 library
include($$PWD/../../hdf5.pri)

# OpenMP library
include($$PWD/../../openmp.pri)

# hdf5helper library
unix|win32: LIBS += -L$$OUT_PWD/../../hdf5helper/ -lhdf5helper

INCLUDEPATH += $$PWD/../../hdf5helper
DEPENDPATH += $$PWD/../../hdf5helper

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../hdf5helper/hdf5helper.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../hdf5helper/libhdf5helper.a
