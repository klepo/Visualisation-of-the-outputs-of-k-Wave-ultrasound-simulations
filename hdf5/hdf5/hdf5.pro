#-------------------------------------------------
#
# Project created by QtCreator 2013-12-24T17:09:56
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = hdf5
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    hdf5file.cpp

HEADERS += hdf5file.h

INCLUDEPATH += "C:/Program Files/HDF_Group/HDF5/1.8.11/include"

LIBS += -L"C:/Program Files/HDF_Group/HDF5/1.8.11/lib" \
        -lhdf5 \
        -lhdf5_cpp \
        -lszip \
        -lzlib \

INCLUDEPATH += "E:/opencv/build/include"

LIBS += -L"E:/opencv/build/x64/vc11/lib" \
        -lopencv_core246 \
        -lopencv_imgproc246 \
        -lopencv_highgui246 \
        -lopencv_contrib246 \
