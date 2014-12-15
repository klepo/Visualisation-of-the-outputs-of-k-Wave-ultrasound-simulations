
# set link type: pkg-config | dynamic | static
LINK_TYPE = pkg-config

win32 {
    INCLUDEPATH += "D:/opencv-2.4.8/opencv/sources/new_build/install/include"
    CONFIG(debug, debug|release) {
        LIBS += -L"D:/opencv-2.4.8/opencv/sources/new_build/install/x64/vc11/staticlib" \
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
        LIBS += -L"D:/opencv-2.4.8/opencv/sources/new_build/install/x64/vc11/staticlib" \
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
    # pkg-config version
    contains(LINK_TYPE, pkg-config) {
        CONFIG += link_pkgconfig
        PKGCONFIG += opencv
    }
    # dynamic version
    else:contains(LINK_TYPE, dynamic) {
        QMAKE_LFLAGS += -Wl,-rpath,/home/petr/opencv-2.4.9/lib
        INCLUDEPATH += "/home/petr/opencv-2.4.9/include"
        LIBS +=  -L"/home/petr/opencv-2.4.9/lib" \
            -lopencv_contrib \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_core \
    }
    # static version (not fully static)
    else:contains(LINK_TYPE, static) {
        QMAKE_LFLAGS += -Wl,-rpath,/home/petr/opencv-2.4.9-static/lib
        INCLUDEPATH += "/home/petr/opencv-2.4.9-static/include"
        LIBS +=  -L"/home/petr/opencv-2.4.9-static/lib" \
            -lopencv_contrib \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_core \
            #-lswscale \
            #-lavutil \
            #-lavformat \
            #-lavcodec \
            #-ldc1394 \
            #-lgthread-2.0 \
            #-lfreetype \
            -lglib-2.0 \
            -lgobject-2.0 \
            #-lfontconfig \
            #-lpango-1.0 \
            #-lcairo \
            #-lgdk_pixbuf-2.0 \
            #-lpangocairo-1.0 \
            #-lpangoft2-1.0 \
            #-lgio-2.0 \
            #-latk-1.0 \
            -lgdk-x11-2.0 \
            -lgtk-x11-2.0 \
            #-lrt \
            -lpthread \
            #-lm \
            #-ldl \
            #-lstdc++
            -lpng \
            -ltiff \
            -lHalf \
            -lIlmImf \
            -ljasper \
            -ljpeg \
            -lz \
    }
    else {error(Wrong LINK_TYPE!)}
}
