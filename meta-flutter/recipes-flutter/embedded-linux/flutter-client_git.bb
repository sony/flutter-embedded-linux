SUMMARY = "Wayland backend (Stand-alone Wayland app)"
DESCRIPTION = "Build the `examples/flutter-wayland-client` project"
AUTHOR = "Hidenori Matsubayashi"
HOMEPAGE = "https://github.com/sony/flutter-embedded-linux"
BUGTRACKER = "https://github.com/sony/flutter-embedded-linux/issues"
LICENSE = "BSD 3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=79ca841e7b9e09b0401186f2aa334adf"

DEPENDS += "wayland \
            wayland-native"

EXTRA_OECMAKE = "-DUSER_PROJECT_PATH=examples/flutter-wayland-client"

inherit embedded-linux

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${WORKDIR}/build/flutter-client ${D}${bindir}
}
