TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS +=  hdf5file-c \
            hdf5file-c-parallel \
            k-wave-h5-processing \
            k-wave-h5-processing-parallel \
            k-wave-h5-visualizer \

hdf5file-c-parallel.file = hdf5file-c/hdf5file-c-parallel.pro
