#-------------------------------------------------
#
# Project created by QtCreator
#
# k-wave-visualizer application
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += console

TARGET = k-wave-visualizer

CONFIG(debug, debug|release) {
    DESTDIR = ../build/$$TARGET/debug
} else {
    DESTDIR = ../build/$$TARGET/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

TEMPLATE = app

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

#PRECOMPILED_HEADER = pch.h

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

include($$PWD/../hdf5file_paths.pri)

# hdf5 library

include($$PWD/../hdf5_paths.pri)

# opencv library

include($$PWD/../opencv_paths.pri)
