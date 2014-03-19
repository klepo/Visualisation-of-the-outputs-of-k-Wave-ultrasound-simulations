#-------------------------------------------------
#
# Project created by QtCreator 2013-12-04T16:48:09
#
#-------------------------------------------------

QT  += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = k-wave-visualizer
TEMPLATE = app

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES +=  main.cpp\
    mainwindow.cpp \
    cvimagewidget.cpp \
    qdoublespinboxts.cpp \
    hdf5readingthread.cpp

HEADERS +=  mainwindow.h \
    cvimagewidget.h \
    qdoublespinboxts.h \
    hdf5readingthread.h

FORMS   +=  mainwindow.ui \


win32 {

    INCLUDEPATH += "E:/opencv-2.4.8/opencv/sources/new_build/install/include"

    CONFIG(debug, debug|release) {

        LIBS += -L"E:/opencv-2.4.8/opencv/sources/new_build/install/x64/vc11/staticlib" \
            -lopencv_core248d \
            -lopencv_contrib248d \
            -lopencv_highgui248d \
            -lopencv_imgproc248d \
#            -lopencv_ml248d \
#            -lopencv_video248d \
#            -lopencv_features2d248d \
#            -lopencv_calib3d248d \
#            -lopencv_objdetect248d \
#            -lopencv_legacy248d \
#            -lopencv_flann248d \
            -lopengl32 \
            -lgdi32 \
            -luser32 \
            -lcomctl32 \
#            -lole32 \
#            -lvfw32 \
            -llibpngd \
            -llibtiffd \
#            -lzlibd \
            -lIlmImfd \
            -llibjasperd \
            -llibjpegd \


    }

    CONFIG(release, release|debug) {

        LIBS += -L"E:/opencv-2.4.8/opencv/sources/new_build/install/x64/vc11/staticlib" \
            -lopencv_core248 \
            -lopencv_imgproc248 \
            -lopencv_highgui248 \
            -lopencv_contrib248 \
            -lopengl32 \
            -lgdi32 \
            -luser32 \
            -lcomctl32 \
#            -lole32 \
#            -lvfw32 \
            -llibpng \
            -llibtiff \
#            -lzlibd \
            -lIlmImf \
            -llibjasper \
            -llibjpeg \
    }

}

unix {

    QMAKE_CXXFLAGS += -std=c++0x

    INCLUDEPATH += "/usr/include/opencv"

    LIBS += -L"/usr/local/lib" \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_contrib \

}

include(../../k-wave-h5-processing/k-wave-h5-processing/hdf5file/hdf5file.pri)


RESOURCES += \
    resources.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../k-wave-h5-processing/build/hdf5file/release/ -lhdf5file
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../k-wave-h5-processing/build/hdf5file/debug/ -lhdf5file
else:unix: LIBS += -L$$PWD/../../k-wave-h5-processing/build/hdf5file/ -lhdf5file

INCLUDEPATH += $$PWD/../../k-wave-h5-processing/k-wave-h5-processing/hdf5file
DEPENDPATH += $$PWD/../../k-wave-h5-processing/k-wave-h5-processing/hdf5file

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../k-wave-h5-processing/build/hdf5file/release/hdf5file.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../k-wave-h5-processing/build/hdf5file/debug/hdf5file.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../k-wave-h5-processing/build/hdf5file/libhdf5file.a
