TEMPLATE = subdirs
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SUBDIRS += hdf5file \
    main

CONFIG += ordered

TARGET = k-wave-h5-processing
