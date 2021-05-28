AUTHOR = "Sony Group Corporation"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=c2c05f9bdd5fc0b458037c2d1fb8d95e"

SRC_URI = "git://chromium.googlesource.com/chromium/tools/depot_tools.git;protocol=https"
SRCREV = "da768751d43b1f287bf99bea703ea13e2eedcf4d"

S = "${WORKDIR}/git"

inherit pkgconfig
# TODO: Add dependent packages.
DEPENDS = "freetype"

GN_TOOLS_PYTHON2_PATH ??= "bootstrap-3.8.0.chromium.8_bin/python/bin"

require gn-args-utils.inc

ENGINE_VERSION ?= "f5b97d0b23a3905e9b5b69aa873afcb52f550aaf"
PACKAGECONFIG ?= "release-mode"
PACKAGECONFIG[debug-mode] = "--runtime-mode debug --unoptimized"
PACKAGECONFIG[profile-mode] = "--runtime-mode profile --no-lto"
PACKAGECONFIG[release-mode] = "--runtime-mode release"

GN_TARGET_OS = "linux"
GN_TARGET_ARCH = "arm64"

GN_ARGS = "--target-sysroot ${STAGING_DIR_TARGET}${PACKAGECONFIG_CONFARGS}"
GN_ARGS_append = " --target-os ${GN_TARGET_OS}"
GN_ARGS_append = " --linux-cpu ${GN_TARGET_ARCH}"
GN_ARGS_append = " --embedder-for-target"
GN_ARGS_append = " --disable-desktop-embeddings"
ARTIFACT_DIR = "${@get_engine_artifact_dir(d)}"

do_configure() {
    # To disable auto update.
    export DEPOT_TOOLS_UPDATE=0
    export PATH=${S}:${S}/${GN_TOOLS_PYTHON2_PATH}:$PATH

    cd ${WORKDIR}
    echo 'solutions = [
        {
            "managed" : False,
            "name" : "src/flutter",
            "url" : "https://github.com/flutter/engine.git@${ENGINE_VERSION}",
            "custom_deps": {},
            "deps_file": "DEPS",
            "safesync_url": "",
            "custom_vars": {
                "download_android_deps": False,
                "download_windows_deps": False,
            },
        },
    ]' > .gclient
    gclient sync

    cd ${WORKDIR}/src
    ./flutter/tools/gn ${GN_ARGS}
}

do_compile() {
    export PATH=${S}:${S}/${GN_TOOLS_PYTHON2_PATH}:$PATH

    cd ${WORKDIR}/src
    ninja -C ${ARTIFACT_DIR}
}

do_install() {
    install -d ${D}${libdir}
    install -m 0755 ${WORKDIR}/src/${ARTIFACT_DIR}/libflutter_engine.so ${D}${libdir}
}

FILES_${PN} = "${libdir}"
FILES_${PN}-dev = "${includedir}"
