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
INCLUDEPATH += $$PWD/processing
DEPENDPATH += $$PWD

# Check Qt version
lessThan(QT_VERSION, 4.8): error(Qt version is too old)

# k-wave-h5-helper library
include($$PWD/../k-wave-h5-helper/k-wave-h5-helper.pri)

SOURCES += \
    main.cpp \
    helper.cpp \
    settings.cpp \
    filescontext.cpp \
    dtsforpcs.cpp \
    paramsdefinition.cpp \
    processing/processing.cpp \
    processing/reshape.cpp \
    processing/changechunks.cpp \
    processing/downsampling.cpp \
    processing/compress.cpp \
    processing/decompress.cpp \
    processing/difference.cpp \

HEADERS += \
    helper.h \
    settings.h \
    filescontext.h \
    dtsforpcs.h \
    paramsdefinition.h \
    processing/processing.h \
    processing/reshape.h \
    processing/changechunks.h \
    processing/downsampling.h \
    processing/compress.h \
    processing/decompress.h \
    processing/difference.h \

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# Copy built file to destination
win32 {
    WDIR = windows-binaries
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../$$WDIR\" $(MKDIR) \"$$PWD/../$$WDIR\") &
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../$$WDIR/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../$$WDIR/$$BUILD_MODE\") &
    QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD/$${TARGET}.exe\" \"$$PWD/../$$WDIR/$$BUILD_MODE/$${TARGET}.exe\" &
}
