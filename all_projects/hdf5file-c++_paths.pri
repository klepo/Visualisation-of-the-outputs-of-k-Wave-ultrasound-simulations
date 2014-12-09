CONFIG(release, debug|release): LIBS += -L$$PWD/build/hdf5file-c++/release/ -lhdf5file-c++
CONFIG(debug, debug|release): LIBS += -L$$PWD/build/hdf5file-c++/debug/ -lhdf5file-c++

INCLUDEPATH += $$PWD/hdf5file-c++
DEPENDPATH += $$PWD/hdf5file-c++

CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c++/release/libhdf5file-c++.a
CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c++/debug/libhdf5file-c++.a

CONFIG(release, debug|release): hdf5file-c++.target = $$PWD/build/hdf5file-c++/release/libhdf5file-c++.a
CONFIG(debug, debug|release): hdf5file-c++.target = $$PWD/build/hdf5file-c++/debug/libhdf5file-c++.a
hdf5file-c++.commands = cd $$PWD/hdf5file-c++ && $(MAKE)
hdf5file-c++.depends = $$PWD/hdf5file-c++/*
QMAKE_EXTRA_TARGETS += hdf5file-c++
