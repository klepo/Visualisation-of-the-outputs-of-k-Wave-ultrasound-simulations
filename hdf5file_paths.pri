win32:CONFIG(release, debug|release): LIBS += -L$$PWD/hdf5file/release/ -lhdf5file
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/hdf5file/debug/ -lhdf5file
else:unix: LIBS += -L$$PWD/hdf5file/ -lhdf5file

INCLUDEPATH += $$PWD/hdf5file
DEPENDPATH += $$PWD/hdf5file

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/hdf5file/release/libhdf5file.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/hdf5file/debug/libhdf5file.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/hdf5file/release/hdf5file.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/hdf5file/debug/hdf5file.lib
else:unix: PRE_TARGETDEPS += $$PWD/hdf5file/libhdf5file.a
