SUMMARY = "Flutter embedding for embedded Linux with DRM-EGLStream backend"
DESCRIPTION = "Build the flutter-drm-eglstream-backend project"

require repository.inc
require dependency.inc

DEPENDS += "libdrm \
            virtual/mesa \
            libinput \
            udev \
            systemd"

EXTRA_OECMAKE = "-DUSER_PROJECT_PATH=examples/flutter-drm-eglstream-backend"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/flutter-drm-eglstream-backend ${D}${bindir}
}

require tasks.inc
