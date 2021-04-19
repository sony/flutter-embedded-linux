// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_LINUXES_EGL_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_LINUXES_EGL_SURFACE_H_

#include <EGL/egl.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"

namespace flutter {

class LinuxesEGLSurface {
 public:
  LinuxesEGLSurface(EGLSurface surface, EGLDisplay display, EGLContext context)
      : surface_(surface), display_(display), context_(context){};

  ~LinuxesEGLSurface() {
    if (surface_ != EGL_NO_SURFACE) {
      if (eglDestroySurface(display_, surface_) != EGL_TRUE) {
        LINUXES_LOG(ERROR) << "Failed to destory surface";
      }
      surface_ = EGL_NO_SURFACE;
    }
  }

  bool IsValid() { return surface_ != EGL_NO_SURFACE; }

  bool MakeCurrent() {
    if (eglMakeCurrent(display_, surface_, surface_, context_) != EGL_TRUE) {
      LINUXES_LOG(ERROR) << "Failed to make the EGL context current: "
                         << get_egl_error_cause();
      return false;
    }
    return true;
  }

  bool SwapBuffers() {
    if (eglSwapBuffers(display_, surface_) != EGL_TRUE) {
      LINUXES_LOG(ERROR) << "Failed to swap the EGL buffer: "
                         << get_egl_error_cause();
      return false;
    }
    return true;
  }

 private:
  EGLDisplay display_;
  EGLSurface surface_;
  EGLContext context_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_LINUXES_EGL_SURFACE_H_