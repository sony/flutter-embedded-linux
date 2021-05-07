SUMMARY = "Flutter embedding for embedded Linux with Wayland backend"
DESCRIPTION = "Build the flutter-wayland-client project"

require repository.inc
require dependency.inc

DEPENDS += "wayland \
            wayland-protocols \
            wayland-native"

EXTRA_OECMAKE = "-DUSER_PROJECT_PATH=examples/flutter-wayland-client"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/flutter-client ${D}${bindir}
}

require tasks.inc
