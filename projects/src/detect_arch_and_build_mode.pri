contains(QMAKE_TARGET.arch, x86_64) {
    ARCH = x64
} else {
    ARCH = x86
}

CONFIG(debug, debug|release) {
    BUILD_MODE = "debug"
}
CONFIG(release, debug|release) {
    BUILD_MODE = "release"
}
