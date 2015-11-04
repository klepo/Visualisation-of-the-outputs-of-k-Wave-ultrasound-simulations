win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../hdf5helper/release/ -lhdf5helper
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../hdf5helper/debug/ -lhdf5helper
else:unix: LIBS += -L$$OUT_PWD/../hdf5helper/ -lhdf5helper

INCLUDEPATH += $$PWD/../hdf5helper
DEPENDPATH += $$PWD/../hdf5helper

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/release/libhdf5helper.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/debug/libhdf5helper.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/release/hdf5helper.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/debug/hdf5helper.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../hdf5helper/libhdf5helper.a
