SUMMARY = "DRM backend"
DESCRIPTION = "Build the `examples/flutter-drm-backend` project"
AUTHOR = "Hidenori Matsubayashi"
HOMEPAGE = "https://github.com/sony/flutter-embedded-linux"
BUGTRACKER = "https://github.com/sony/flutter-embedded-linux/issues"
LICENSE = "BSD 3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=79ca841e7b9e09b0401186f2aa334adf"

DEPENDS += "libdrm \
            virtual/mesa \
            libinput \
            udev \
            systemd"

EXTRA_OECMAKE = "-DUSER_PROJECT_PATH=examples/flutter-drm-backend"

inherit embedded-linux

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${WORKDIR}/build/flutter-drm-backend ${D}${bindir}
}

