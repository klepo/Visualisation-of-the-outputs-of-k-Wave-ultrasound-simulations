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
    \ #h5-rename \
    k-wave-h5-helper \
    \ #k-wave-h5-helper-parallel \
    k-wave-h5-processing \
    \ #k-wave-h5-processing-parallel \
    rewrite-ram \
    \ #tests \

greaterThan(QT_VERSION, 5) {
    greaterThan(QT_MINOR_VERSION, 2) {
        SUBDIRS += \
            k-wave-h5-visualizer \
            \ #k-wave-h5-processing-gui
    }
}
