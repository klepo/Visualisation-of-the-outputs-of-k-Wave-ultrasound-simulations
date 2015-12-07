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

# hdf5file library

include($$PWD/../hdf5helper/hdf5helper.pri)

# hdf5 library

include($$PWD/../hdf5.pri)

# opencv library

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

FORMS += \
    mainwindow.ui \
    dialog.ui \

RESOURCES += \
    resources.qrc \

OTHER_FILES += \
    fragmentShader.frag \
    vertexShader.vert \

win32:RC_ICONS = $$PWD/icons/images/icons_33.ico
