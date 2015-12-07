win32|unix: LIBS += -L$$OUT_PWD/../hdf5helper/ -lhdf5helper

INCLUDEPATH += $$PWD/../hdf5helper
DEPENDPATH += $$PWD/../hdf5helper

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/hdf5helper.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/libhdf5helper.a

hdf5helperMakefile.target = $$OUT_PWD/../hdf5helper/Makefile
CONFIG(debug, debug|release) {
    hdf5helperMakefile.commands = cd $$PWD/../hdf5helper/ && $(QMAKE) -spec $$basename(QMAKESPEC) CONFIG+=debug -o $$OUT_PWD/../hdf5helper/Makefile
}
CONFIG(release, debug|release) {
    hdf5helperMakefile.commands = cd $$PWD/../hdf5helper/ && $(QMAKE) -spec $$basename(QMAKESPEC) -o $$OUT_PWD/../hdf5helper/Makefile
}

win32:!win32-g++:hdf5helper.target = $$OUT_PWD/../hdf5helper/hdf5helper.lib
else:unix|win32-g++:hdf5helper.target = $$OUT_PWD/../hdf5helper/libhdf5helper.a
hdf5helper.commands = cd $$OUT_PWD/../hdf5helper/ && $(MAKE)
hdf5helper.depends = hdf5helperMakefile

QMAKE_EXTRA_TARGETS += hdf5helperMakefile hdf5helper
