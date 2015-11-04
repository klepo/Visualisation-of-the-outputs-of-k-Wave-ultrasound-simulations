#-------------------------------------------------
#
# Project created by QtCreator
#
# k-wave-h5-visualizer application
#
#-------------------------------------------------

QT += core gui widgets

CONFIG += console

TARGET = k-wave-h5-visualizer

#CONFIG(debug, debug|release) {
#    DESTDIR = ../build/$$TARGET/debug
#} else {
#    DESTDIR = ../build/$$TARGET/release
#}

#OBJECTS_DIR = $$DESTDIR/.obj
#MOC_DIR = $$DESTDIR/.moc
#RCC_DIR = $$DESTDIR/.qrc
#UI_DIR = $$DESTDIR/.ui

TEMPLATE = app

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES +=  main.cpp\
    mainwindow.cpp \
    cvimagewidget.cpp \
    qdoublespinboxts.cpp \
    hdf5readingthread.cpp \
    openglwindow.cpp \
    gwindow.cpp \
    openedh5file.cpp \
    h5objecttovisualize.cpp \
    h5subobjecttovisualize.cpp

HEADERS +=  mainwindow.h \
    cvimagewidget.h \
    qdoublespinboxts.h \
    hdf5readingthread.h \
    openglwindow.h \
    gwindow.h \
    openedh5file.h \
    h5objecttovisualize.h \
    h5subobjecttovisualize.h

FORMS +=  mainwindow.ui \
    dialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    fragmentShader.frag \
    vertexShader.vert

unix:QMAKE_CXXFLAGS += -std=c++0x

# hdf5file library

include($$PWD/../hdf5helper/hdf5helper_paths.pri)

# hdf5 library

include($$PWD/../hdf5-c_paths.pri)

# opencv library

include($$PWD/../opencv_paths.pri)
