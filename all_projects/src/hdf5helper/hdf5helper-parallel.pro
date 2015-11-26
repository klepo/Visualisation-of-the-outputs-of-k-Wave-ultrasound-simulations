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
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = hdf5helper-parallel

QMAKE_LFLAGS += /ignore:4099

DEFINES += PARALLEL_HDF5

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# hdf5 library
include($$PWD/../hdf5-c-parallel_paths.pri)

SOURCES += HDF5File.cpp HDF5Vector3D.cpp HDF5Object.cpp HDF5Dataset.cpp HDF5Group.cpp HDF5Attribute.cpp

HEADERS += hdf5helperparallel.h HDF5File.h HDF5Vector3D.h HDF5Object.h HDF5Dataset.h HDF5Group.h HDF5Attribute.h
