TEMPLATE = subdirs

SUBDIRS +=  \#hdf5file-c \
            \#hdf5file-c++ \
            hdf5file-c-parallel \

CONFIG  +=  ordered

SUBDIRS +=  \#k-wave-h5-visualizer \
            k-wave-h5-processing \
