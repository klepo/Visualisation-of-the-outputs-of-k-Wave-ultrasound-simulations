#-------------------------------------------------
#
# Project created by QtCreator
#
# k-wave-h5-processing-parallel console application
#
#-------------------------------------------------

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = k-wave-h5-processing-parallel

win32:QMAKE_LFLAGS += /ignore:4099

win32 {
    LIBS += -lws2_32 #gethostname
}

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# hdf5file library
include($$PWD/../hdf5helper-parallel/hdf5helper-parallel.pri)

# hdf5 library
include($$PWD/../hdf5-parallel.pri)

# opencv library
include($$PWD/../opencv.pri)

SOURCES += k-wave-h5-processing-parallel.cpp
