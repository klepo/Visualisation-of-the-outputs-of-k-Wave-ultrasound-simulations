win32 {
    QMAKE_CXXFLAGS += -openmp
}

unix {
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
    LIBS += -fopenmp
}
