do_configure_prepend() {
    install -d ${S}/build
    install -m 0644 ${STAGING_LIBDIR}/libflutter_engine.so ${S}/build/
}
do_configure_prepend[depends] += "flutter-engine:do_populate_sysroot"

FILES_${PN} = "${bindir}"
