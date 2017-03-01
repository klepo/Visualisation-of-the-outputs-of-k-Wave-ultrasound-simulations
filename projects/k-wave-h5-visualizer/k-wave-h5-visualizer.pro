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

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    qdoublespinboxts.cpp \
    hdf5readingthread.cpp \
    openglwindow.cpp \
    gwindow.cpp \
    openedh5file.cpp \
    h5objecttovisualize.cpp \
    h5subobjecttovisualize.cpp \
    qvector3di.cpp \
    colormap.cpp \
    imagewidget.cpp

HEADERS += \
    mainwindow.h \
    qdoublespinboxts.h \
    hdf5readingthread.h \
    openglwindow.h \
    vertices.h \
    gwindow.h \
    openedh5file.h \
    h5objecttovisualize.h \
    h5subobjecttovisualize.h \
    qvector3di.h \
    colormap.h \
    imagewidget.h

FORMS += \
    mainwindow.ui \
    dialog.ui \

RESOURCES += \
    resources.qrc \

OTHER_FILES += \
    shaders/fragmentShader.frag \
    shaders/vertexShader.vert \

RC_FILE = appicon.rc

# Copy built file to destination
win32 {
    WDIR = windows-binaries
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR\" $(MKDIR) \"$$PWD/../../$$WDIR\") &
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../../$$WDIR/$$BUILD_MODE\") &
    QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD/$${TARGET}.exe\" \"$$PWD/../../$$WDIR/$$BUILD_MODE/$${TARGET}.exe\" &
}
