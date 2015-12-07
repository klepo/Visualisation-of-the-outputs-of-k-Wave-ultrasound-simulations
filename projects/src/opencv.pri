include($$PWD/opencv_paths.pri)

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
