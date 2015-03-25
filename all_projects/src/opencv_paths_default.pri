
OPENCV_INCLUDE_DIR = "D:/opencv-2.4.8/opencv/build/include"
OPENCV_LIBS_DIR = "D:/opencv-2.4.8/opencv/build/x64/vc11/lib"

#OPENCV_INCLUDE_DIR = "/home/petr/opencv-2.4.9/include"
#OPENCV_LIBS_DIR = "/home/petr/opencv-2.4.9/lib"

win32 {
    INCLUDEPATH += $$OPENCV_INCLUDE_DIR
    CONFIG(debug, debug|release) {
        LIBS += -L$$OPENCV_LIBS_DIR \
            -lopencv_core248d \
            -lopencv_contrib248d \
            -lopencv_highgui248d \
            -lopencv_imgproc248d \
    }

    CONFIG(release, debug|release) {
        LIBS += -L$$OPENCV_LIBS_DIR \
            -lopencv_core248 \
            -lopencv_contrib248 \
            -lopencv_highgui248 \
            -lopencv_imgproc248 \
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
