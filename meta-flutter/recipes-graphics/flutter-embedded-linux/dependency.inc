TOOLCHAIN = "clang"
DEPENDS += "libxkbcommon  \
            virtual/libgles2 \
            virtual/egl \
            flutter-engine"

inherit pkgconfig cmake features_check
