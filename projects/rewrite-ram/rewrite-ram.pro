#-------------------------------------------------
#
# Project created by QtCreator
#
# rewrite-ram console application
#
#-------------------------------------------------

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= debug_and_release
CONFIG -= qt
CONFIG += c++11

TARGET = rewrite-ram

win32:QMAKE_LFLAGS += /ignore:4099

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# k-wave-h5-helper library
include($$PWD/../k-wave-h5-helper/k-wave-h5-helper.pri)

SOURCES += rewrite-ram.cpp

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# Copy built file to destination
win32 {
    WDIR = windows-binaries
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR\" $(MKDIR) \"$$PWD/../../$$WDIR\") &
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../../$$WDIR/$$BUILD_MODE\") &
    QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD/$${TARGET}.exe\" \"$$PWD/../../$$WDIR/$$BUILD_MODE/$${TARGET}.exe\" &
}
