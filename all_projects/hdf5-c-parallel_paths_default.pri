
# set link type: pkg-config | dynamic | static
LINK_TYPE = dynamic

win32 {

    INCLUDEPATH += "c:\Program Files\MPICH2\include"
    LIBS += -L"c:\Program Files\MPICH2\lib" \
        -lmpi \
        -lcxx \

    INCLUDEPATH += "d:\HDF5-1.8.14-win64-parallel\include"
    CONFIG(debug, debug|release) {
        LIBS += -L"d:\HDF5-1.8.14-win64-parallel\lib" \
            -llibhdf5_D \
            #-llibzlib_D \
            -llibszip_D
    }
    CONFIG(release, debug|release) {
        LIBS += -L"d:\HDF5-1.8.14-win64-parallel\lib" \
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

        QMAKE_CC = mpiCC                       # replace gcc with mpicc
        QMAKE_CXX = mpicxx                     # replace g++ with mpicxx
        QMAKE_LINK = mpiCC                     # change the linker, if not set it is g++

        INCLUDEPATH += "/usr/local/hdf5-1.8.13-parallel/include"
        LIBS += -L"/usr/local/hdf5-1.8.13-parallel/lib" \
            -lhdf5 \
            -lz \
            -ldl \
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
