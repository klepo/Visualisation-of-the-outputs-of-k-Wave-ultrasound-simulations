CONFIG(release, debug|release): LIBS += -L$$PWD/build/hdf5file/release/ -lhdf5file
CONFIG(debug, debug|release): LIBS += -L$$PWD/build/hdf5file/debug/ -lhdf5file

INCLUDEPATH += $$PWD/hdf5file
DEPENDPATH += $$PWD/hdf5file

CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file/release/libhdf5file.a
CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file/debug/libhdf5file.a

CONFIG(release, debug|release): hdf5file.target = $$PWD/build/hdf5file/release/libhdf5file.a
CONFIG(debug, debug|release): hdf5file.target = $$PWD/build/hdf5file/debug/libhdf5file.a
CONFIG(release, debug|release): hdf5file.commands = cd $$PWD/hdf5file && qmake && $(MAKE) clean && $(MAKE)
CONFIG(debug, debug|release): hdf5file.commands = cd $$PWD/hdf5file && qmake CONFIG+=debug && $(MAKE) clean && $(MAKE)
#hdf5file.depends = $$PWD/hdf5file/Makefile
QMAKE_EXTRA_TARGETS += hdf5file

