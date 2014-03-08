#-------------------------------------------------
#
# Project created by QtCreator 2013-12-24T17:09:56
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = hdf5
CONFIG   += console
CONFIG   -= app_bundle

#QMAKE_CFLAGS_DEBUG += -MTd
#QMAKE_CFLAGS_RELEASE += -MT
#QMAKE_CXXFLAGS_DEBUG += -MTd
#QMAKE_CXXFLAGS_RELEASE += -MT

#QMAKE_CFLAGS_DEBUG -= -MDd
#QMAKE_CFLAGS_RELEASE -= -MD
#QMAKE_CXXFLAGS_DEBUG -= -MDd
#QMAKE_CXXFLAGS_RELEASE -= -MD

TEMPLATE = app

SOURCES += main.cpp \
    hdf5file/hdf5file.cpp

HEADERS += hdf5file/hdf5file.h

win32 {
    INCLUDEPATH += "E:/HDF5-1.8.12-win64-static-with-zlib_szip/include"
    INCLUDEPATH += "E:/opencv-2.4.8/opencv/build/include"

    CONFIG(debug, debug|release) {
        LIBS += -L"E:/HDF5-1.8.12-win64-static-with-zlib_szip/lib" \
            -llibhdf5_D \
            -llibhdf5_cpp_D \
            -llibzlib_D \
            -llibszip_D


        LIBS += -L"E:/opencv-2.4.8/opencv/build/x64/vc11/lib" \
            -lopencv_core248d \
            -lopencv_contrib248d \
            -lopencv_highgui248d \
            -lopencv_imgproc248d \
    }

    CONFIG(release, release|debug) {
        LIBS += -L"E:/HDF5-1.8.12-win64-static-with-zlib_szip/lib" \
            -llibhdf5 \
            -llibhdf5_cpp \
            -llibzlib \
            -llibszip

        LIBS += -L"E:/opencv-2.4.8/opencv/build/x64/vc11/lib" \
            -lopencv_core248 \
            -lopencv_imgproc248 \
            -lopencv_highgui248 \
            -lopencv_contrib248
    }

    LIBS += -lvfw32 \
        -lcomctl32

}

unix {

    QMAKE_CXXFLAGS += -std=c++0x

    INCLUDEPATH += "/usr/local/hdf5-1.8.11-serial/include"

    LIBS += -L"/usr/local/hdf5-1.8.11-serial/lib" \
        -lhdf5 \
        -lhdf5_cpp \
        -lz \
        -ldl \

    INCLUDEPATH += "/usr/include/opencv"

    LIBS += -L"/usr/local/lib" \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_contrib \
}
