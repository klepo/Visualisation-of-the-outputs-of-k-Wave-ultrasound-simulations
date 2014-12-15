CONFIG(release, debug|release): LIBS += -L$$PWD/../build/hdf5file-c++/release/ -lhdf5file-c++
CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/hdf5file-c++/debug/ -lhdf5file-c++

INCLUDEPATH += $$PWD/../hdf5file-c++
DEPENDPATH += $$PWD/../hdf5file-c++

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/hdf5file-c++/release/libhdf5file-c++.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/hdf5file-c++/debug/libhdf5file-c++.a

else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/hdf5file-c++/release/hdf5file-c++.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/hdf5file-c++/debug/hdf5file-c++.lib

else:unix: CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/hdf5file-c++/release/libhdf5file-c++.a
else:unix: CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/hdf5file-c++/debug/libhdf5file-c++.a
