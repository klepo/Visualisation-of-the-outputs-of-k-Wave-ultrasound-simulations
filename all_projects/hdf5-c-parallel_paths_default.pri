
MPI_INCLUDE_DIR = "C:/Program Files/MPICH2/include"
MPI_LIBS_DIR = "C:/Program Files/MPICH2/lib"

HDF5_INCLUDE_DIR = "D:/HDF5-1.8.14-win64-parallel/include"
HDF5_LIBS_DIR = "D:/HDF5-1.8.14-win64-parallel/lib"

#HDF5_INCLUDE_DIR = "/usr/local/hdf5-1.8.13-parallel/include"
#HDF5_LIBS_DIR = "/usr/local/hdf5-1.8.13-parallel/lib"

win32 {

    INCLUDEPATH += $$MPI_INCLUDE_DIR
    LIBS += -L$$MPI_LIBS_DIR \
        -lmpi \
        -lcxx \

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
