
OPENCV_INCLUDE_DIR = "F:/opencv/opencv/build/include"
OPENCV_LIBS_DIR = "F:/opencv/opencv/build/x64/vc12/lib"

#OPENCV_INCLUDE_DIR = "/home/petr/opencv-2.4.9/include"
#OPENCV_LIBS_DIR = "/home/petr/opencv-2.4.9/lib"

win32 {
    INCLUDEPATH += $$OPENCV_INCLUDE_DIR
    CONFIG(debug, debug|release) {
        LIBS += -L$$OPENCV_LIBS_DIR \
            -lopencv_ts300d \
            -lopencv_world300d \
    }

    CONFIG(release, debug|release) {
        LIBS += -L$$OPENCV_LIBS_DIR \
            -lopencv_ts300 \
            -lopencv_world300 \
    }
}

unix {
    QMAKE_LFLAGS += -Wl,-rpath,$$OPENCV_LIBS_DIR
    INCLUDEPATH += $$OPENCV_INCLUDE_DIR
    LIBS +=  -L$$OPENCV_LIBS_DIR \
        -lopencv_contrib \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_core \
}
