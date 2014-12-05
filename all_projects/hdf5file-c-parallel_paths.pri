CONFIG(release, debug|release): LIBS += -L$$PWD/build/hdf5file-c-parallel/release/ -lhdf5file-c-parallel
CONFIG(debug, debug|release): LIBS += -L$$PWD/build/hdf5file-c-parallel/debug/ -lhdf5file-c-parallel

INCLUDEPATH += $$PWD/hdf5file-c-parallel
DEPENDPATH += $$PWD/hdf5file-c-parallel

CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c-parallel/release/libhdf5file-c-parallel.a
CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c-parallel/debug/libhdf5file-c-parallel.a

CONFIG(release, debug|release): hdf5file-c-parallel.target = $$PWD/build/hdf5file-c-parallel/release/libhdf5file-c-parallel.a
CONFIG(debug, debug|release): hdf5file-c-parallel.target = $$PWD/build/hdf5file-c-parallel/debug/libhdf5file-c-parallel.a
hdf5file-c-parallel.commands = cd $$PWD/hdf5file-c-parallel && $(MAKE)
hdf5file-c-parallel.depends = $$PWD/hdf5file-c-parallel/*
QMAKE_EXTRA_TARGETS += hdf5file-c-parallel

