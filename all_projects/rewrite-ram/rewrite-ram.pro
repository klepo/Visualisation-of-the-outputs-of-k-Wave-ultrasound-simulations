#-------------------------------------------------
#
# Project created by QtCreator
#
# rewrite-ram console application
#
#-------------------------------------------------

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += rewrite-ram.cpp

TARGET = rewrite-ram

CONFIG(debug, debug|release) {
    DESTDIR = ../build/$$TARGET/debug
} else {
    DESTDIR = ../build/$$TARGET/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

# hdf5 library

include($$PWD/../hdf5-c-parallel_paths.pri)
