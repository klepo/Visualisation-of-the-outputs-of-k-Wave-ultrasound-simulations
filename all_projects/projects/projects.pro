TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS +=  hdf5helper \
            hdf5helper-parallel \
            k-wave-h5-processing \
            k-wave-h5-processing-parallel \
            k-wave-h5-visualizer \

hdf5helper-parallel.file = hdf5helper/hdf5helper-parallel.pro
hdf5helper-parallel.target = hdf5helper-parallel
