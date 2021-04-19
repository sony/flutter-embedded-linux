SUMMARY = "Flutter embedding for embedded Linux with DRM-GBM backend"
DESCRIPTION = "Build the flutter-drm-gbm-backend project"
AUTHOR = "Sony Group Corporation"
HOMEPAGE = "https://github.com/sony/flutter-embedded-linux"
BUGTRACKER = "https://github.com/sony/flutter-embedded-linux/issues"

DEPENDS += "libdrm \
            virtual/mesa \
            libinput \
            udev \
            systemd"

EXTRA_OECMAKE = "-DUSER_PROJECT_PATH=examples/flutter-drm-gbm-backend"

include flutter-embedded-linux.inc

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/flutter-drm-gbm-backend ${D}${bindir}
}

