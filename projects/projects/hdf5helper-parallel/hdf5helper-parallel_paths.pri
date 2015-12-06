win32: LIBS += -L$$OUT_PWD/../hdf5helper-parallel/ -lhdf5helper-parallel
else:unix: LIBS += -L$$OUT_PWD/../hdf5helper-parallel/ -lhdf5helper-parallel

INCLUDEPATH += $$PWD/../hdf5helper-parallel
DEPENDPATH += $$PWD/../hdf5helper-parallel
INCLUDEPATH += $$PWD/../hdf5helper
DEPENDPATH += $$PWD/../hdf5helper

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper-parallel/libhdf5helper-parallel.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper-parallel/hdf5helper-parallel.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper-parallel/libhdf5helper-parallel.a
