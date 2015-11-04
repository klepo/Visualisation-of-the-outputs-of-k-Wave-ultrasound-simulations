#-------------------------------------------------
#
# Project created by QtCreator
#
# hdf5helper-parallel library
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt

TARGET = hdf5helper-parallel

#CONFIG(debug, debug|release) {
#    DESTDIR = ../build/$$TARGET/debug
#} else {
#    DESTDIR = ../build/$$TARGET/release
#}

#OBJECTS_DIR = $$DESTDIR/.obj
#MOC_DIR = $$DESTDIR/.moc
#RCC_DIR = $$DESTDIR/.qrc
#UI_DIR = $$DESTDIR/.ui

#QMAKE_CFLAGS_DEBUG += -MTd
#QMAKE_CFLAGS_RELEASE += -MT
#QMAKE_CXXFLAGS_DEBUG += -MTd
#QMAKE_CXXFLAGS_RELEASE += -MT

#QMAKE_CFLAGS_DEBUG -= -MDd
#QMAKE_CFLAGS_RELEASE -= -MD
#QMAKE_CXXFLAGS_DEBUG -= -MDd
#QMAKE_CXXFLAGS_RELEASE -= -MD

unix:QMAKE_CXXFLAGS += -std=c++0x

DEFINES += PARALLEL_HDF5

SOURCES += HDF5File.cpp HDF5Vector3D.cpp HDF5Object.cpp HDF5Dataset.cpp HDF5Group.cpp HDF5Attribute.cpp

HEADERS += hdf5helperparallel.h HDF5File.h HDF5Vector3D.h HDF5Object.h HDF5Dataset.h HDF5Group.h HDF5Attribute.h

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# hdf5 library

include($$PWD/../hdf5-c-parallel_paths.pri)
