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

# Check Qt version
lessThan(QT_VERSION, 5.2): error(Qt version is too old)

# k-wave-h5-helper library
include($$PWD/../k-wave-h5-helper/k-wave-h5-helper.pri)

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    qdoublespinboxts.cpp \
    openglwindow.cpp \
    gwindow.cpp \
    qvector3di.cpp \
    colormap.cpp \
    imagewidget.cpp \
    h5objecttovisualize.cpp \
    h5openedfile.cpp \
    h5readingthread.cpp \
    datasetinfogroupbox.cpp \
    fileinfodockwidget.cpp \
    opacitygroupbox.cpp \
    datasetsdockwidget.cpp \
    abstractfilewidget.cpp \
    abstractobjectwidget.cpp \
    abstractwidget.cpp \
    slicedockwidget.cpp \
    minmaxgroupbox.cpp

HEADERS += \
    mainwindow.h \
    qdoublespinboxts.h \
    openglwindow.h \
    vertices.h \
    gwindow.h \
    qvector3di.h \
    colormap.h \
    imagewidget.h \
    h5objecttovisualize.h \
    h5openedfile.h \
    h5readingthread.h \
    datasetinfogroupbox.h \
    fileinfodockwidget.h \
    opacitygroupbox.h \
    datasetsdockwidget.h \
    abstractfilewidget.h \
    abstractobjectwidget.h \
    abstractwidget.h \
    slicedockwidget.h \
    minmaxgroupbox.h

FORMS += \
    mainwindow.ui \
    dialog.ui \
    datasetinfogroupbox.ui \
    fileinfodockwidget.ui \
    opacitygroupbox.ui \
    datasetsdockwidget.ui \
    slicedockwidget.ui \
    minmaxgroupbox.ui

RESOURCES += \
    resources.qrc \

OTHER_FILES += \
    shaders/fragmentshader.frag \
    shaders/vertexshader.vert \

RC_FILE = appicon.rc

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# Copy built file to destination
win32 {
    WDIR = windows-binaries
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR\" $(MKDIR) \"$$PWD/../../$$WDIR\") &
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../../$$WDIR/$$BUILD_MODE\") &
    QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD/$${TARGET}.exe\" \"$$PWD/../../$$WDIR/$$BUILD_MODE/$${TARGET}.exe\" &
}
