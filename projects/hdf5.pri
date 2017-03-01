exists($$PWD/hdf5_paths.pri) {
    include($$PWD/hdf5_paths.pri)
} else {
    include($$PWD/hdf5_paths_default.pri)
}

win32 {
    INCLUDEPATH += $$HDF5_INCLUDE_DIR
    CONFIG(debug, debug|release) {
        LIBS += -L$$HDF5_LIBS_DIR \
            -llibhdf5_D \
            #-llibzlib_D \
            #-llibszip_D
    }

    CONFIG(release, debug|release) {
        LIBS += -L$$HDF5_LIBS_DIR \
            -llibhdf5 \
            #-llibzlib \
            #-llibszip
    }
}

unix {
    QMAKE_LFLAGS += -Wl,-rpath,$$HDF5_LIBS_DIR
    INCLUDEPATH += $$HDF5_INCLUDE_DIR
    LIBS += -L$$HDF5_LIBS_DIR \
        -lhdf5 \
        -lz \
        -ldl \
}
