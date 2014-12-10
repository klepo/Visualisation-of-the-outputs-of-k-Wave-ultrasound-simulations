CONFIG(release, debug|release): LIBS += -L$$PWD/build/hdf5file-c-parallel/release/ -lhdf5file-c-parallel
CONFIG(debug, debug|release): LIBS += -L$$PWD/build/hdf5file-c-parallel/debug/ -lhdf5file-c-parallel

INCLUDEPATH += $$PWD/hdf5file-c-parallel
DEPENDPATH += $$PWD/hdf5file-c-parallel

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c-parallel/release/libhdf5file-c-parallel.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c-parallel/debug/libhdf5file-c-parallel.a

else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c-parallel/release/hdf5file-c-parallel.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c-parallel/debug/hdf5file-c-parallel.lib

else:unix: CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c-parallel/release/libhdf5file-c-parallel.a
else:unix: CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build/hdf5file-c-parallel/debug/libhdf5file-c-parallel.a

#####

win32-g++:CONFIG(release, debug|release): hdf5file-c-parallel.target = $$PWD/build/hdf5file-c-parallel/release/libhdf5file-c-parallel.a
else:win32-g++:CONFIG(debug, debug|release): hdf5file-c-parallel.target = $$PWD/build/hdf5file-c-parallel/debug/libhdf5file-c-parallel.a

else:win32:!win32-g++:CONFIG(release, debug|release): hdf5file-c-parallel.target = $$PWD/build/hdf5file-c-parallel/release/hdf5file-c-parallel.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): hdf5file-c-parallel.target = $$PWD/build/hdf5file-c-parallel/debug/hdf5file-c-parallel.lib

else:unix: CONFIG(release, debug|release): hdf5file-c-parallel.target = $$PWD/build/hdf5file-c-parallel/release/libhdf5file-c-parallel.a
else:unix: CONFIG(debug, debug|release): hdf5file-c-parallel.target = $$PWD/build/hdf5file-c-parallel/debug/libhdf5file-c-parallel.a

hdf5file-c-parallel.commands = $(MAKE) -C $$PWD/hdf5file-c-parallel
hdf5file-c-parallel.depends = $$PWD/hdf5file-c-parallel/*
QMAKE_EXTRA_TARGETS += hdf5file-c-parallel

