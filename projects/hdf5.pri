exists($$PWD/hdf5_paths.pri) {
    include($$PWD/hdf5_paths.pri)
} else {
    include($$PWD/hdf5_paths_default.pri)
}

win32 {
    INCLUDEPATH += $$HDF5_INCLUDE_DIR
    CONFIG(debug, debug|release) {
        LIBS += -L$$HDF5_LIBS_DIR \
            -llibhdf5_D

        exists($$HDF5_LIBS_DIR/libzlib_D.lib) {
                LIBS += -llibzlib_D
        }

        exists($$HDF5_LIBS_DIR/libszip_D.lib) {
                LIBS += -llibszip_D
        }
    }

    CONFIG(release, debug|release) {
        LIBS += -L$$HDF5_LIBS_DIR \
            -llibhdf5

        exists($$HDF5_LIBS_DIR/libzlib.lib) {
                LIBS += -llibzlib
        }

        exists($$HDF5_LIBS_DIR/libszip.lib) {
                LIBS += -llibszip
        }
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
