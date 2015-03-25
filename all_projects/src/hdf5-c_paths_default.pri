
HDF5_INCLUDE_DIR = "D:/HDF5-1.8.12-win64-static-with-zlib_szip-threadsafe/include"
HDF5_LIBS_DIR = "D:/HDF5-1.8.12-win64-static-with-zlib_szip-threadsafe/lib"

#HDF5_INCLUDE_DIR = "/usr/local/hdf5-1.8.13-serial-static/include"
#HDF5_LIBS_DIR = "/usr/local/hdf5-1.8.13-serial-static/lib"

win32 {
    INCLUDEPATH += $$HDF5_INCLUDE_DIR
    CONFIG(debug, debug|release) {
        LIBS += -L$$HDF5_LIBS_DIR \
            -llibhdf5_D \
            -llibzlib_D \
            -llibszip_D
    }

    CONFIG(release, debug|release) {
        LIBS += -L$$HDF5_LIBS_DIR \
            -llibhdf5 \
            -llibzlib \
            -llibszip
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
