#-------------------------------------------------
#
# Project created by QtCreator 2013-12-04T16:48:09
#
#-------------------------------------------------

QT  += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
TEMPLATE = app

SOURCES +=  main.cpp\
    mainwindow.cpp \
    loadingthread.cpp \
    dataset.cpp \
    cvimagewidget.cpp

HEADERS +=  mainwindow.h \
    dataset.h \
    loadingthread.h \
    cvimagewidget.h

FORMS   +=  mainwindow.ui

INCLUDEPATH += "E:/opencv/build/include" \
    .

LIBS += -L"E:/opencv/build/x64/vc11/lib" \
    -lopencv_core246 \
    -lopencv_imgproc246 \
    #-lopencv_highgui246 \
    -lopencv_contrib246 \

INCLUDEPATH += "C:/Program Files/HDF_Group/HDF5/1.8.11/include"

LIBS += -L"C:/Program Files/HDF_Group/HDF5/1.8.11/lib" \
    -lhdf5 \
    -lhdf5_cpp \
    -lszip \
    -lzlib \

RESOURCES += \
    resources.qrc
