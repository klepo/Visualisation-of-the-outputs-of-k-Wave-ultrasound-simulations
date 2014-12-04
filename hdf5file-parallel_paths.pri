CONFIG(release, debug|release): LIBS += -L$$PWD/build/hdf5file-parallel/release/ -lhdf5file-parallel
CONFIG(debug, debug|release): LIBS += -L$$PWD/build/hdf5file-parallel/debug/ -lhdf5file-parallel

INCLUDEPATH += $$PWD/hdf5file-parallel
DEPENDPATH += $$PWD/hdf5file-parallel

CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-parallel/release/libhdf5file-parallel.a
CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-parallel/debug/libhdf5file-parallel.a

CONFIG(release, debug|release): hdf5file-parallel.target = $$PWD/build/hdf5file-parallel/release/libhdf5file-parallel.a
CONFIG(debug, debug|release): hdf5file-parallel.target = $$PWD/build/hdf5file-parallel/debug/libhdf5file-parallel.a
CONFIG(release, debug|release): hdf5file-parallel.commands = cd $$PWD/hdf5file-parallel && qmake && $(MAKE) clean && $(MAKE)
CONFIG(debug, debug|release): hdf5file-parallel.commands = cd $$PWD/hdf5file-parallel && qmake CONFIG+=debug && $(MAKE) clean && $(MAKE)
#hdf5file-parallel.depends = $$PWD/hdf5file/Makefile
QMAKE_EXTRA_TARGETS += hdf5file-parallel

