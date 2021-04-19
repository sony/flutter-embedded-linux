SUMMARY = "Flutter embedding for embedded Linux with Wayland backend"
DESCRIPTION = "Build the flutter-wayland-client project"
AUTHOR = "Sony Group Corporation"
HOMEPAGE = "https://github.com/sony/flutter-embedded-linux"
BUGTRACKER = "https://github.com/sony/flutter-embedded-linux/issues"

DEPENDS += "wayland \
            wayland-native"

EXTRA_OECMAKE = "-DUSER_PROJECT_PATH=examples/flutter-wayland-client"

include flutter-embedded-linux.inc

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/flutter-client ${D}${bindir}
}
