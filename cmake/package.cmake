cmake_minimum_required(VERSION 3.10)

find_package(PkgConfig)

# common libraries.
pkg_check_modules(EGL REQUIRED egl)

# requires for supporting keyboard inputs.
pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)

# depends on backend type.
if(${BACKEND_TYPE} MATCHES "DRM-(GBM|EGLSTREAM)")
  # DRM backend
  pkg_check_modules(DRM REQUIRED libdrm)
  pkg_check_modules(LIBINPUT REQUIRED libinput)
  pkg_check_modules(LIBUDEV REQUIRED libudev)
  pkg_check_modules(LIBSYSTEMD REQUIRED libsystemd)
  if(${BACKEND_TYPE} STREQUAL "DRM-GBM")
    pkg_check_modules(GBM REQUIRED gbm)
  endif()
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
elseif(${BACKEND_TYPE} STREQUAL "X11")
  pkg_check_modules(X11 REQUIRED x11)
else()
  # Wayland backend
  pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
  pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client>=1.16.0)
  pkg_check_modules(WAYLAND_CURSOR REQUIRED wayland-cursor>=1.16.0)
  pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl>=1.16.0)
endif()

# requires for supporting external texture plugin.
# OpenGL ES3 are included in glesv2.
pkg_check_modules(GLES REQUIRED glesv2)
