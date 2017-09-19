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
    \ #tests \
    h5-rename \

greaterThan(QT_VERSION, 5.2) {
    SUBDIRS += \
        k-wave-h5-visualizer \
        \ #k-wave-h5-processing-gui
}
