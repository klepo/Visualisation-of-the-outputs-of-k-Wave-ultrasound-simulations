win32|unix: LIBS += -L$$OUT_PWD/../k-wave-h5-helper/ -lk-wave-h5-helper

INCLUDEPATH += $$PWD/../k-wave-h5-helper
DEPENDPATH += $$PWD/../k-wave-h5-helper

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../k-wave-h5-helper/k-wave-h5-helper.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../k-wave-h5-helper/libk-wave-h5-helper.a

k-wave-h5-helperMakefile.target = $$OUT_PWD/../k-wave-h5-helper/Makefile
CONFIG(debug, debug|release) {
    k-wave-h5-helperMakefile.commands = cd $$PWD/../k-wave-h5-helper/ && $(QMAKE) -spec $$basename(QMAKESPEC) CONFIG+=debug -o $$OUT_PWD/../k-wave-h5-helper/Makefile
}
CONFIG(release, debug|release) {
    k-wave-h5-helperMakefile.commands = cd $$PWD/../k-wave-h5-helper/ && $(QMAKE) -spec $$basename(QMAKESPEC) -o $$OUT_PWD/../k-wave-h5-helper/Makefile
}

win32:!win32-g++:k-wave-h5-helper.target = $$OUT_PWD/../k-wave-h5-helper/k-wave-h5-helper.lib
else:unix|win32-g++:k-wave-h5-helper.target = $$OUT_PWD/../k-wave-h5-helper/libk-wave-h5-helper.a
k-wave-h5-helper.commands = cd "$$OUT_PWD/"../k-wave-h5-helper/ && $(MAKE)
k-wave-h5-helper.depends = k-wave-h5-helperMakefile

QMAKE_EXTRA_TARGETS += k-wave-h5-helperrMakefile k-wave-h5-helper

# HDF5 library
include($$PWD/../hdf5.pri)

# OpenMP library
include($$PWD/../openmp.pri)
