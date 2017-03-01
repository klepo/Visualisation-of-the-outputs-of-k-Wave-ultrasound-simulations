# Detect architecture and build mode (ARCH = x86/x64, BUILD_MODE = debug/release)
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
