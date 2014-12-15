
# set link type: pkg-config | dynamic | static
LINK_TYPE = dynamic

win32 {
    INCLUDEPATH += "D:/HDF5-1.8.12-win64-serial-threadsafe/include"
    CONFIG(debug, debug|release) {
        LIBS += -L"D:/HDF5-1.8.12-win64-serial-threadsafe/lib" \
            -llibhdf5_D \
            #-llibzlib_D \
            -llibszip_D
    }
    CONFIG(release, debug|release) {
        LIBS += -L"D:/HDF5-1.8.12-win64-serial-threadsafe/lib" \
            -llibhdf5 \
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
            -lz \
            -ldl
    }
    # static version
    else:contains(LINK_TYPE, static) {
        QMAKE_LFLAGS += -Wl,-rpath,/usr/local/hdf5-1.8.13-serial-static/lib
        INCLUDEPATH += "/usr/local/hdf5-1.8.13-serial-static/include"
        LIBS += -L"/usr/local/hdf5-1.8.13-serial-static/lib" \
            -lhdf5 \
            -lz \
            -ldl \
    }
    else {error(Wrong LINK_TYPE!)}

}
