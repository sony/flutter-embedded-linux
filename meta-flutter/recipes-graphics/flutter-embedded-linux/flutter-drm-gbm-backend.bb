SUMMARY = "Flutter embedding for embedded Linux with DRM-GBM backend"
DESCRIPTION = "Build the flutter-drm-gbm-backend project"

require repository.inc
require dependency.inc

DEPENDS += "libdrm \
            virtual/mesa \
            libinput \
            udev \
            systemd"

EXTRA_OECMAKE = "-DUSER_PROJECT_PATH=examples/flutter-drm-gbm-backend"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/flutter-drm-gbm-backend ${D}${bindir}
}

require tasks.inc
