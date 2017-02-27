exists($$PWD/hdf5-parallel_paths.pri) {
    include($$PWD/hdf5-parallel_paths.pri)
} else {
    include($$PWD/hdf5-parallel_paths_default.pri)
}

win32 {

    INCLUDEPATH += $$MPI_INCLUDE_DIR
    LIBS += -L$$MPI_LIBS_DIR \
        -lmsmpi \

    INCLUDEPATH += $$HDF5_INCLUDE_DIR
    CONFIG(debug, debug|release) {
        LIBS += -L$$HDF5_LIBS_DIR \
            -llibhdf5_D \
            -llibzlib_D \
            -llibszip_D \
    }

    CONFIG(release, debug|release) {
        LIBS += -L$$HDF5_LIBS_DIR \
            -llibhdf5 \
            -llibzlib \
            -llibszip \
    }
}

unix {
    QMAKE_LFLAGS += -Wl,-rpath,$$HDF5_LIBS_DIR

    QMAKE_CC = mpiCC        # replace gcc with mpicc
    QMAKE_CXX = mpicxx      # replace g++ with mpicxx
    QMAKE_LINK = mpiCC      # change the linker, if not set it is g++

    INCLUDEPATH += $$HDF5_INCLUDE_DIR
    LIBS += -L$$HDF5_LIBS_DIR \
        -lhdf5 \
        -lz \
        -ldl \
}
