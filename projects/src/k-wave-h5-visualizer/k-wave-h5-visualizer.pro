#-------------------------------------------------
#
# Project created by QtCreator
#
# k-wave-h5-visualizer application
#
#-------------------------------------------------

QT += widgets
TEMPLATE = app
CONFIG += console
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = k-wave-h5-visualizer

win32:QMAKE_LFLAGS += /ignore:4099

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# hdf5helper library
include($$PWD/../hdf5helper/hdf5helper.pri)

# HDF5 library
include($$PWD/../hdf5.pri)

# OpenCV library
include($$PWD/../opencv.pri)

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    cvimagewidget.cpp \
    qdoublespinboxts.cpp \
    hdf5readingthread.cpp \
    openglwindow.cpp \
    gwindow.cpp \
    openedh5file.cpp \
    h5objecttovisualize.cpp \
    h5subobjecttovisualize.cpp \
    qvector3di.cpp

HEADERS += \
    mainwindow.h \
    cvimagewidget.h \
    qdoublespinboxts.h \
    hdf5readingthread.h \
    openglwindow.h \
    gwindow.h \
    openedh5file.h \
    h5objecttovisualize.h \
    h5subobjecttovisualize.h \
    qvector3di.h

FORMS += \
    mainwindow.ui \
    dialog.ui \

RESOURCES += \
    resources.qrc \

OTHER_FILES += \
    fragmentShader.frag \
    vertexShader.vert \

win32:RC_ICONS = $$PWD/icons/images/icons_33.ico
