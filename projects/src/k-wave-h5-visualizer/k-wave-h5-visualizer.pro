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
    qvector3di.cpp \
    colormap.cpp

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
    qvector3di.h \
    colormap.h

FORMS += \
    mainwindow.ui \
    dialog.ui \

RESOURCES += \
    resources.qrc \

OTHER_FILES += \
    fragmentShader.frag \
    vertexShader.vert \

win32:RC_ICONS = $$PWD/icons/images/icons_33.ico

# Copy built files to destination
#QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../install/$$ARCH\" $(MKDIR) \"$$PWD/../../install/$$ARCH\") &
#QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../install/$$ARCH/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../../install/$$ARCH/$$BUILD_MODE\") &
#QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD\\$$BUILD_MODE\\$${TARGET}.lib\" \"$$PWD\\lib\\$$ARCH\\$$BUILD_MODE\\$${TARGET}.lib\" &
#QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD\\$${TARGET}.exe\" \"$$PWD\\..\\..\\install\\$$ARCH\\$$BUILD_MODE\\$${TARGET}.exe\" &

QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../install\" $(MKDIR) \"$$PWD/../../install\") &
QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../install/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../../install/$$BUILD_MODE\") &
#QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD\\$$BUILD_MODE\\$${TARGET}.lib\" \"$$PWD\\lib\\$$ARCH\\$$BUILD_MODE\\$${TARGET}.lib\" &
QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD\\$${TARGET}.exe\" \"$$PWD\\..\\..\\install\\$$BUILD_MODE\\$${TARGET}.exe\" &

