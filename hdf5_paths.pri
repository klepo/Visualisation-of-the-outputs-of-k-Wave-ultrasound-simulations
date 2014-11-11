
# set link type: pkg-config | dynamic | static
LINK_TYPE = dynamic

win32 {
    INCLUDEPATH += "D:/HDF5-1.8.12-win64-static-with-zlib_szip-threadsafe/include"
    CONFIG(debug, debug|release) {
        LIBS += -L"D:/HDF5-1.8.12-win64-static-with-zlib_szip-threadsafe/lib" \
            -llibhdf5_D \
            -llibhdf5_cpp_D \
            #-llibzlib_D \
            -llibszip_D
    }
    CONFIG(release, debug|release) {
        LIBS += -L"D:/HDF5-1.8.12-win64-static-with-zlib_szip-threadsafe/lib" \
            -llibhdf5 \
            -llibhdf5_cpp \
            #-llibzlib \
            -llibszip
    }
}

unix {

    # pkg-config version
    contains(LINK_TYPE, pkg-config) {
        CONFIG += link_pkgconfig
        PKGCONFIG += hdf5
    }
    # dynamic version
    else:contains(LINK_TYPE, dynamic) {
        QMAKE_LFLAGS += -Wl,-rpath,/usr/local/hdf5-1.8.13-serial/lib
        INCLUDEPATH += "/usr/local/hdf5-1.8.13-serial/include"
        LIBS += -L"/usr/local/hdf5-1.8.13-serial/lib" \
            -lhdf5 \
            -lhdf5_cpp \
            -lz \
            -ldl
    }
    # static version (not yet static)
    else:contains(LINK_TYPE, static) {
        QMAKE_LFLAGS += -Wl,-rpath,/usr/local/hdf5-1.8.13-serial/lib
        INCLUDEPATH += "/usr/local/hdf5-1.8.13-serial/include"
        LIBS += -L"/usr/local/hdf5-1.8.13-serial/lib" \
            -lhdf5 \
            -lhdf5_cpp \
            -lz \
            -ldl \
    }
    else {error(Wrong LINK_TYPE!)}

}
