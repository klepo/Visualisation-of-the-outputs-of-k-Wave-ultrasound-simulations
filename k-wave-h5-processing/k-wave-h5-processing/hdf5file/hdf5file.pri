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

    QMAKE_CXXFLAGS += -std=c++0x
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/hdf5-1.8.11-serial/lib
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/hdf5-1.8.11-serial/lib
    INCLUDEPATH += "/usr/local/hdf5-1.8.11-serial/include"

    LIBS += -L"/usr/local/hdf5-1.8.11-serial/lib" \
        -lhdf5 \
        -lhdf5_cpp \
        -lz \
        -ldl

}
