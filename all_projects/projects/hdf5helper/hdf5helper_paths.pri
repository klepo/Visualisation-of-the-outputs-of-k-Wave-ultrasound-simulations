win32: LIBS += -L$$OUT_PWD/../hdf5helper/ -lhdf5helper
else:unix: LIBS += -L$$OUT_PWD/../hdf5helper/ -lhdf5helper

INCLUDEPATH += $$PWD/../hdf5helper
DEPENDPATH += $$PWD/../hdf5helper

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/libhdf5helper.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/hdf5helper.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/libhdf5helper.a
