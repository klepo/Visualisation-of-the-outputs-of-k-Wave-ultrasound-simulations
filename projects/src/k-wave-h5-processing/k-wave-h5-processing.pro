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

# hdf5file library
include($$PWD/../hdf5helper/hdf5helper.pri)

# hdf5 library
include($$PWD/../hdf5.pri)

# opencv library
include($$PWD/../opencv.pri)

SOURCES += k-wave-h5-processing.cpp
