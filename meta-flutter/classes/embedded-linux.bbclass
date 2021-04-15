SUMMARY = "Common parts of the recipe for flutter-embedded-linux"
AUTHOR = "Hidenori Matsubayashi"
HOMEPAGE = "https://github.com/sony/flutter-embedded-linux"
BUGTRACKER = "https://github.com/sony/flutter-embedded-linux/issues"
LICENSE = "BSD 3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=79ca841e7b9e09b0401186f2aa334adf"

SRC_URI = "git://github.com/HidenoriMatsubayashi/flutter-embedded-linux.git;protocol=https"
SRCREV = "${AUTOREV}"
S = "${WORKDIR}/git"
TOOLCHAIN = "clang"
DEPENDS += "libxkbcommon  \
            virtual/egl \
            flutter-engine"

RDEPENDS_${PN} = "flutter-engine"
FILES_${PN} = "${bindir}"

inherit pkgconfig cmake features_check

do_configure_prepend() {
    install -d ${S}/build
    install -m 0644 ${STAGING_LIBDIR}/libflutter_engine.so ${S}/build/
}
do_configure_prepend[depends] += "flutter-engine:do_populate_sysroot"

