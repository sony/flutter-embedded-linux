// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/elinux_egl_surface.h"

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"

namespace flutter {

ELinuxEGLSurface::ELinuxEGLSurface(EGLSurface surface,
                                   EGLDisplay display,
                                   EGLContext context)
    : surface_(surface), display_(display), context_(context){};

ELinuxEGLSurface::~ELinuxEGLSurface() {
  if (surface_ != EGL_NO_SURFACE) {
    if (eglDestroySurface(display_, surface_) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "Failed to destory surface: "
                        << get_egl_error_cause();
    }
    surface_ = EGL_NO_SURFACE;
  }
}

bool ELinuxEGLSurface::IsValid() const {
  return surface_ != EGL_NO_SURFACE;
}

bool ELinuxEGLSurface::MakeCurrent() const {
  if (eglMakeCurrent(display_, surface_, surface_, context_) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to make the EGL context current: "
                      << get_egl_error_cause();
    return false;
  }

#if defined(ENABLE_EGL_ASYNC_BUFFER_SWAPPING)
  // Non-blocking when swappipping buffers on Wayland.
  // However, we might encounter rendering problems on some Wayland compositors
  // (e.g. weston 9.0) when we use them.
  // See also: 
  //   - https://github.com/sony/flutter-embedded-linux/issues/230
  //   - https://github.com/sony/flutter-embedded-linux/issues/234
  //   - https://github.com/sony/flutter-embedded-linux/issues/220
  if (eglSwapInterval(display_, 0) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to eglSwapInterval(Free): "
                      << get_egl_error_cause();
  }
#endif

  return true;
}

bool ELinuxEGLSurface::SwapBuffers() const {
  if (eglSwapBuffers(display_, surface_) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to swap the EGL buffer: "
                      << get_egl_error_cause();
    return false;
  }
  return true;
}

}  // namespace flutter
