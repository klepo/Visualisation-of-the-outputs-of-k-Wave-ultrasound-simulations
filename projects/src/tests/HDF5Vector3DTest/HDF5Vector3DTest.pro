#-------------------------------------------------
#
# Project created by QtCreator 2016-06-29T14:30:13
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_hdf5vector3dtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_hdf5vector3dtest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

# hdf5file library
#include($$PWD/../../hdf5helper/hdf5helper.pri)

unix:!macx|win32: LIBS += -L$$OUT_PWD/../../hdf5helper-parallel/ -lhdf5helper-parallel

INCLUDEPATH += $$PWD/../../hdf5helper
DEPENDPATH += $$PWD/../../hdf5helper

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../hdf5helper/hdf5helper.lib
else:unix:!macx|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../hdf5helper/libhdf5helper.a


# HDF5 library
include($$PWD/../../hdf5.pri)
