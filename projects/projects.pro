#-------------------------------------------------
#
# Project created by QtCreator
#
# Main project with subdirs
#
#-------------------------------------------------

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS +=  \
    hdf5helper \
    \ #hdf5helper-parallel \
    k-wave-h5-processing \
    \ #k-wave-h5-processing-parallel \
    k-wave-h5-visualizer \
    \ #tests \
    \ #k-wave-h5-processing-gui
