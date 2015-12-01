win32|unix: LIBS += -L$$OUT_PWD/../hdf5helper/ -lhdf5helper

INCLUDEPATH += $$PWD/../hdf5helper
DEPENDPATH += $$PWD/../hdf5helper

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/hdf5helper.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/libhdf5helper.a
