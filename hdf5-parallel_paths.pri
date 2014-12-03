
# set link type: pkg-config | dynamic | static
LINK_TYPE = dynamic

win32 {
    INCLUDEPATH += "D:/HDF5-1.8.12-win64-static-with-zlib_szip-threadsafe/include"
    CONFIG(debug, debug|release) {
        LIBS += -L"D:/HDF5-1.8.12-win64-static-with-zlib_szip-threadsafe/lib" \
            -llibhdf5_cpp_D \
            -llibhdf5_D \
            #-llibzlib_D \
            -llibszip_D
    }
    CONFIG(release, debug|release) {
        LIBS += -L"D:/HDF5-1.8.12-win64-static-with-zlib_szip-threadsafe/lib" \
            -llibhdf5_cpp \
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
    # dynamic version - only this is functional yet
    else:contains(LINK_TYPE, dynamic) {
        QMAKE_LFLAGS += -Wl,-rpath,/usr/local/hdf5-1.8.13-parallel/lib
        INCLUDEPATH += "/usr/local/hdf5-1.8.13-parallel/include"
        LIBS += -L"/usr/local/hdf5-1.8.13-parallel/lib" \
            -lhdf5 \
            -lz \
            -ldl

        #INCLUDEPATH += . /usr/include/     # add include path here to find the header
        LIBS += -pthread -lmpi -ldl -lhwloc # add library and path here in the link stage
        QMAKE_CC = mpicc                    # replace gcc with mpicc
        QMAKE_CXX = mpicxx                  # replace g++ with mpicxx
        QMAKE_LINK = mpicc                  # change the linker, if not set it is g++

    }
    # static version
    else:contains(LINK_TYPE, static) {
        QMAKE_LFLAGS += -Wl,-rpath,/usr/local/hdf5-1.8.13-serial-static/lib
        INCLUDEPATH += "/usr/local/hdf5-1.8.13-serial-static/include"
        LIBS += -L"/usr/local/hdf5-1.8.13-serial-static/lib" \
            -lhdf5_cpp \
            -lhdf5 \
            -lz \
            -ldl \
    }
    else {error(Wrong LINK_TYPE!)}

}
