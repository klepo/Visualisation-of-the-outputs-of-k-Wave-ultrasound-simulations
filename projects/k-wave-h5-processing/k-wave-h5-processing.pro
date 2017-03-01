#-------------------------------------------------
#
# Project created by QtCreator
#
# k-wave-h5-processing console application
#
#-------------------------------------------------

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = k-wave-h5-processing

win32:QMAKE_LFLAGS += /ignore:4099

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# hdf5helper library
include($$PWD/../hdf5helper/hdf5helper.pri)

SOURCES += \
    helper.cpp \
    settings.cpp \
    filescontext.cpp \
    dtsforpcs.cpp \
    paramsdefinition.cpp \
    processing.cpp \
    main.cpp

HEADERS += \
    helper.h \
    settings.h \
    filescontext.h \
    dtsforpcs.h \
    paramsdefinition.h \
    processing.h

# Copy built file to destination
win32 {
    WDIR = windows-binaries
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR\" $(MKDIR) \"$$PWD/../../$$WDIR\") &
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../../$$WDIR/$$BUILD_MODE\") &
    QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD/$${TARGET}.exe\" \"$$PWD/../../$$WDIR/$$BUILD_MODE/$${TARGET}.exe\" &
}
