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
INCLUDEPATH += $$PWD/widgets
DEPENDPATH += $$PWD
DEPENDPATH += $$PWD/widgets

# Check Qt version
lessThan(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 2) {
    error(Qt version is too old)
}

# k-wave-h5-helper library
include($$PWD/../k-wave-h5-helper/k-wave-h5-helper.pri)

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    widgets/qdoublespinboxts.cpp \
    openglwindow.cpp \
    gwindow.cpp \
    qvector3di.cpp \
    colormap.cpp \
    widgets/imagewidget.cpp \
    h5objecttovisualize.cpp \
    h5openedfile.cpp \
    h5readingthread.cpp \
    widgets/datasetinfogroupbox.cpp \
    widgets/fileinfodockwidget.cpp \
    widgets/opacitygroupbox.cpp \
    widgets/datasetsdockwidget.cpp \
    widgets/abstractwidget.cpp \
    widgets/slicedockwidget.cpp \
    widgets/minmaxgroupbox.cpp \
    widgets/selecteddatasetdockwidget.cpp \
    widgets/window3ddockwidget.cpp \
    widgets/settings3ddockwidget.cpp \
    widgets/timeseriescontrolgroupbox.cpp \

HEADERS += \
    mainwindow.h \
    widgets/qdoublespinboxts.h \
    openglwindow.h \
    vertices.h \
    gwindow.h \
    qvector3di.h \
    colormap.h \
    widgets/imagewidget.h \
    h5objecttovisualize.h \
    h5openedfile.h \
    h5readingthread.h \
    widgets/datasetinfogroupbox.h \
    widgets/fileinfodockwidget.h \
    widgets/opacitygroupbox.h \
    widgets/datasetsdockwidget.h \
    widgets/abstractwidget.h \
    widgets/slicedockwidget.h \
    widgets/minmaxgroupbox.h \
    widgets/selecteddatasetdockwidget.h \
    widgets/settings3ddockwidget.h \
    widgets/window3ddockwidget.h \
    widgets/timeseriescontrolgroupbox.h \

FORMS += \
    mainwindow.ui \
    widgets/datasetinfogroupbox.ui \
    widgets/fileinfodockwidget.ui \
    widgets/opacitygroupbox.ui \
    widgets/datasetsdockwidget.ui \
    widgets/slicedockwidget.ui \
    widgets/minmaxgroupbox.ui \
    widgets/selecteddatasetdockwidget.ui \
    widgets/settings3ddockwidget.ui \
    widgets/window3ddockwidget.ui \
    widgets/timeseriescontrolgroupbox.ui \

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
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../$$WDIR\" $(MKDIR) \"$$PWD/../$$WDIR\") &
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../$$WDIR/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../$$WDIR/$$BUILD_MODE\") &
    QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD/$${TARGET}.exe\" \"$$PWD/../$$WDIR/$$BUILD_MODE/$${TARGET}.exe\" &
}
