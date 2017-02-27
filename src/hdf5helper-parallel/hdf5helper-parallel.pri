# Old not updated nonfunctional!

win32|unix: LIBS += -L$$OUT_PWD/../hdf5helper-parallel/ -lhdf5helper-parallel

INCLUDEPATH += $$PWD/../hdf5helper-parallel
INCLUDEPATH += $$PWD/../hdf5helper
DEPENDPATH += $$PWD/../hdf5helper-parallel
DEPENDPATH += $$PWD/../hdf5helper

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper-parallel/hdf5helper-parallel.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper-parallel/libhdf5helper-parallel.a

hdf5helper-parallelMakefile.target = $$OUT_PWD/../hdf5helper-parallel/Makefile
CONFIG(debug, debug|release) {
    hdf5helper-parallelMakefile.commands = cd $$PWD/../hdf5helper-parallel/ && $(QMAKE) -spec $$basename(QMAKESPEC) CONFIG+=debug -o $$OUT_PWD/../hdf5helper-parallel/Makefile
}
CONFIG(release, debug|release) {
    hdf5helper-parallelMakefile.commands = cd $$PWD/../hdf5helper-parallel/ && $(QMAKE) -spec $$basename(QMAKESPEC) -o $$OUT_PWD/../hdf5helper-parallel/Makefile
}

win32:!win32-g++:hdf5helper-parallel.target = $$OUT_PWD/../hdf5helper-parallel/hdf5helper-parallel.lib
else:unix|win32-g++:hdf5helper-parallel.target = $$OUT_PWD/../hdf5helper-parallel/libhdf5helper-parallel.a
hdf5helper-parallel.commands = cd $$OUT_PWD/../hdf5helper-parallel/ && $(MAKE)
hdf5helper-parallel.depends = hdf5helper-parallelMakefile

QMAKE_EXTRA_TARGETS += hdf5helper-parallelMakefile hdf5helper-parallel
