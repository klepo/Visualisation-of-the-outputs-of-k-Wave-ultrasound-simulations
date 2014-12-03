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

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += k-wave-h5-processing.cpp

TARGET = k-wave-h5-processing

CONFIG(debug, debug|release) {
    DESTDIR = ../build/$$TARGET/debug
} else {
    DESTDIR = ../build/$$TARGET/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

#QMAKE_CXXFLAGS_WARN_ON -= -W3
#QMAKE_CXXFLAGS_WARN_ON += -W4

#QMAKE_CFLAGS_WARN_ON -= -W3
#QMAKE_CFLAGS_WARN_ON += -W4

#QMAKE_CFLAGS_WARN_ON += -Wall
#QMAKE_CXXFLAGS_WARN_ON += -Wall

unix:QMAKE_CXXFLAGS += -std=c++0x

# hdf5file library

include($$PWD/../hdf5file-parallel_paths.pri)

# hdf5 library

include($$PWD/../hdf5-parallel_paths.pri)

# opencv library

include($$PWD/../opencv_paths.pri)
