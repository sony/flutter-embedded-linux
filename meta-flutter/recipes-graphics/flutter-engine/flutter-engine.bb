AUTHOR = "Sony Group Corporation"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=c2c05f9bdd5fc0b458037c2d1fb8d95e"

SRC_URI = "git://chromium.googlesource.com/chromium/tools/depot_tools.git;protocol=https"
SRCREV = "da768751d43b1f287bf99bea703ea13e2eedcf4d"

S = "${WORKDIR}/git"

inherit pkgconfig
DEPENDS = "freetype libx11 gtk+3"

GN_TOOLS_PYTHON2_PATH ??= "bootstrap-3.8.0.chromium.8_bin/python/bin"

require gn-utils.inc
GN_ARGS = "--target-sysroot ${PKG_CONFIG_SYSROOT_DIR}"
GN_ARGS_append = " --target-os ${TARGET_OS}"
GN_ARGS_append = " --linux-cpu ${@gn_target_arch_name(d)}"
GN_ARGS_append = " --runtime-mode release"
GN_ARGS_append = " --embedder-for-target"
OUT_DIR = "out/${TARGET_OS}_release_${@gn_target_arch_name(d)}"

do_configure() {
    export DEPOT_TOOLS_UPDATE=0
    export PATH=${S}:${S}/${GN_TOOLS_PYTHON2_PATH}:$PATH

    cd ${WORKDIR}
    echo 'solutions = [
        {
            "managed" : False,
            "name" : "src/flutter",
            "url" : "https://github.com/flutter/engine.git",
            "custom_deps": {},
            "deps_file": "DEPS",
            "safesync_url": "",
        },
    ]' > .gclient
    gclient sync

    cd ${WORKDIR}/src
    ./flutter/tools/gn ${GN_ARGS}
    
}

do_compile() {
    export PATH=${S}:${S}/${GN_TOOLS_PYTHON2_PATH}:$PATH

    cd ${WORKDIR}/src
    ninja -C ${OUT_DIR}
}

do_install() {
    install -d ${D}${libdir}
    install -m 0755 ${WORKDIR}/src/${OUT_DIR}/libflutter_engine.so ${D}${libdir}
}

FILES_${PN} = "${libdir}"
FILES_${PN}-dev = "${includedir}"
