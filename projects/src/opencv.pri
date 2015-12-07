exists($$PWD/opencv_paths.pri) {
    include($$PWD/opencv_paths.pri)
} else {
    include($$PWD/opencv_paths_default.pri)
}

QMAKE_CLEAN += "*.dll"

win32 {
    INCLUDEPATH += $$OPENCV_INCLUDE_DIR
    equals(OPENCV_WORLD, 1) {
        CONFIG(debug, debug|release) {
            LIBS += -L$$OPENCV_LIBS_DIR \
                -lopencv_ts$${OPENCV_VERSION}d \
                -lopencv_world$${OPENCV_VERSION}d \

            # Copy OpenCV *.dll to destination dir
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_world$${OPENCV_VERSION}d.dll\" \"$$OUT_PWD\\opencv_world$${OPENCV_VERSION}d.dll\" &
        }
        CONFIG(release, debug|release) {
            LIBS += -L$$OPENCV_LIBS_DIR \
                -lopencv_ts$${OPENCV_VERSION} \
                -lopencv_world$${OPENCV_VERSION} \

            # Copy OpenCV *.dll to destination dir
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_world$${OPENCV_VERSION}.dll\" \"$$OUT_PWD\\opencv_world$${OPENCV_VERSION}.dll\" &
        }
    } else {
        CONFIG(debug, debug|release) {
            LIBS += -L$$OPENCV_LIBS_DIR \
                -lopencv_highgui$${OPENCV_VERSION}d \
                -lopencv_imgproc$${OPENCV_VERSION}d \
                -lopencv_imgcodecs$${OPENCV_VERSION}d \
                -lopencv_core$${OPENCV_VERSION}d \

            # Copy OpenCV *.dll to destination dir
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_highgui$${OPENCV_VERSION}d.dll\" \"$$OUT_PWD\\opencv_highgui$${OPENCV_VERSION}d.dll\" &
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_imgproc$${OPENCV_VERSION}d.dll\" \"$$OUT_PWD\opencv_imgproc$${OPENCV_VERSION}d.dll\" &
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_imgcodecs$${OPENCV_VERSION}d.dll\" \"$$OUT_PWD\\opencv_imgcodecs$${OPENCV_VERSION}d.dll\" &
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_videoio$${OPENCV_VERSION}d.dll\" \"$$OUT_PWD\\opencv_videoio$${OPENCV_VERSION}d.dll\" &
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_core$${OPENCV_VERSION}d.dll\" \"$$OUT_PWD\\opencv_core$${OPENCV_VERSION}d.dll\" &
        }
        CONFIG(release, debug|release) {
            LIBS += -L$$OPENCV_LIBS_DIR \
                -lopencv_highgui$${OPENCV_VERSION} \
                -lopencv_imgproc$${OPENCV_VERSION} \
                -lopencv_imgcodecs$${OPENCV_VERSION} \
                -lopencv_core$${OPENCV_VERSION} \

            # Copy OpenCV *.dll to destination dir
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_highgui$${OPENCV_VERSION}.dll\" \"$$OUT_PWD\\opencv_highgui$${OPENCV_VERSION}.dll\" &
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_imgproc$${OPENCV_VERSION}.dll\" \"$$OUT_PWD\opencv_imgproc$${OPENCV_VERSION}.dll\" &
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_imgcodecs$${OPENCV_VERSION}.dll\" \"$$OUT_PWD\\opencv_imgcodecs$${OPENCV_VERSION}.dll\" &
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_videoio$${OPENCV_VERSION}.dll\" \"$$OUT_PWD\\opencv_videoio$${OPENCV_VERSION}.dll\" &
            QMAKE_POST_LINK += $${QMAKE_COPY} \"$$OPENCV_BIN_DIR\\opencv_core$${OPENCV_VERSION}.dll\" \"$$OUT_PWD\\opencv_core$${OPENCV_VERSION}.dll\" &
        }
    }
}

unix {
    QMAKE_LFLAGS += -Wl,-rpath,$$OPENCV_LIBS_DIR
    INCLUDEPATH += $$OPENCV_INCLUDE_DIR
    LIBS +=  -L$$OPENCV_LIBS_DIR \
        -lopencv_highgui \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_core \
}
