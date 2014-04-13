TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += main.cpp

TARGET = k-wave-h5-processing

#QMAKE_CXXFLAGS_WARN_ON -= -W3
#QMAKE_CXXFLAGS_WARN_ON += -W4

#QMAKE_CFLAGS_WARN_ON -= -W3
#QMAKE_CFLAGS_WARN_ON += -W4

#QMAKE_CFLAGS_WARN_ON += -Wall
#QMAKE_CXXFLAGS_WARN_ON += -Wall

win32 {

    INCLUDEPATH += "G:/opencv-2.4.8/opencv/sources/new_build/install/include"

    CONFIG(debug, debug|release) {

        LIBS += -L"G:/opencv-2.4.8/opencv/sources/new_build/install/x64/vc11/staticlib" \
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
            -lzlibd \
            -lIlmImfd \
            -llibjasperd \
            -llibjpegd \


    }

    CONFIG(release, debug|release) {

        LIBS += -L"G:/opencv-2.4.8/opencv/sources/new_build/install/x64/vc11/staticlib" \
            -lopencv_core248 \
            -lopencv_contrib248 \
            -lopencv_highgui248 \
            -lopencv_imgproc248 \
            -lopengl32 \
            -lgdi32 \
            -luser32 \
            -lcomctl32 \
#            -lole32 \
#            -lvfw32 \
            -llibpng \
            -llibtiff \
            -lzlib \
            -lIlmImf \
            -llibjasper \
            -llibjpeg \
    }

}

unix {

    QMAKE_CXXFLAGS += -std=c++0x #`pkg-config --static --cflags --libs opencv`

    INCLUDEPATH += "/usr/include/opencv"

    LIBS += -L"/usr/local/lib" \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_contrib \

}

include(../hdf5file/hdf5file.pri)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../hdf5file/release/ -lhdf5file
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../hdf5file/debug/ -lhdf5file
else:unix: LIBS += -L$$OUT_PWD/../hdf5file/ -lhdf5file

INCLUDEPATH += $$PWD/../hdf5file
DEPENDPATH += $$PWD/../hdf5file

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../hdf5file/release/hdf5file.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../hdf5file/debug/hdf5file.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../hdf5file/libhdf5file.a
