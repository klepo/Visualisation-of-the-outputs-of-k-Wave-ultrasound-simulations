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
CONFIG -= debug_and_release
CONFIG -= qt
CONFIG += c++11

TARGET = rewrite-ram

win32:QMAKE_LFLAGS += /ignore:4099

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# k-wave-h5-helper library
include($$PWD/../k-wave-h5-helper/k-wave-h5-helper.pri)

SOURCES += rewrite-ram.cpp
