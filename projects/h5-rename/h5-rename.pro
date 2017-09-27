#-------------------------------------------------
#
# Project created by QtCreator
#
# h5-rename console application
#
#-------------------------------------------------

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release
CONFIG += c++11

TARGET = h5-rename

win32:QMAKE_LFLAGS += /ignore:4099

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Check Qt version
lessThan(QT_VERSION, 4.8): error(Qt version is too old)

# hdf5helper library
include($$PWD/../hdf5helper/hdf5helper.pri)

SOURCES += \
    main.cpp \
    settings.cpp \
    ../k-wave-h5-processing/paramsdefinition.cpp \
    ../k-wave-h5-processing/helper.cpp \

HEADERS += \
    settings.h \
    ../k-wave-h5-processing/paramsdefinition.h \
    ../k-wave-h5-processing/helper.h \

# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
include($$PWD/../detect_arch_and_build_mode.pri)

# Copy built file to destination
win32 {
    WDIR = windows-binaries
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR\" $(MKDIR) \"$$PWD/../../$$WDIR\") &
    QMAKE_POST_LINK += ($(CHK_DIR_EXISTS) \"$$PWD/../../$$WDIR/$$BUILD_MODE\" $(MKDIR) \"$$PWD/../../$$WDIR/$$BUILD_MODE\") &
    QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OUT_PWD/$${TARGET}.exe\" \"$$PWD/../../$$WDIR/$$BUILD_MODE/$${TARGET}.exe\" &
}
