TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt

TARGET = hdf5file

#QMAKE_CFLAGS_DEBUG += -MTd
#QMAKE_CFLAGS_RELEASE += -MT
#QMAKE_CXXFLAGS_DEBUG += -MTd
#QMAKE_CXXFLAGS_RELEASE += -MT

#QMAKE_CFLAGS_DEBUG -= -MDd
#QMAKE_CFLAGS_RELEASE -= -MD
#QMAKE_CXXFLAGS_DEBUG -= -MDd
#QMAKE_CXXFLAGS_RELEASE -= -MD

SOURCES += hdf5file.cpp

HEADERS += hdf5file.h

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

include(hdf5file.pri)
