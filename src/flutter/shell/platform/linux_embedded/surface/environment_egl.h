// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_H_

#include <EGL/egl.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"

namespace flutter {

class EnvironmentEgl {
 public:
  EnvironmentEgl(EGLNativeDisplayType platform_display)
      : display_(EGL_NO_DISPLAY), sub_environment_(false), valid_(false) {
    display_ = eglGetDisplay(platform_display);
    if (display_ == EGL_NO_DISPLAY) {
      ELINUX_LOG(ERROR) << "Failed to get the EGL display: "
                        << get_egl_error_cause();
      return;
    }

    valid_ = InitializeEgl();
  }

  EnvironmentEgl() : display_(EGL_NO_DISPLAY), valid_(false) {}

  virtual ~EnvironmentEgl() {
    if (display_ != EGL_NO_DISPLAY && !sub_environment_) {
      if (eglTerminate(display_) != EGL_TRUE) {
        ELINUX_LOG(ERROR) << "Failed to terminate the EGL display: "
                          << get_egl_error_cause();
      }
      display_ = EGL_NO_DISPLAY;
    }
  }

  bool InitializeEgl() const {
    if (eglInitialize(display_, nullptr, nullptr) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "Failed to initialize the EGL display: "
                        << get_egl_error_cause();
      return false;
    }

    if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "Failed to bind EGL API: " << get_egl_error_cause();
      return false;
    }

    return true;
  }

  bool IsValid() const { return valid_; }

  EGLDisplay Display() const { return display_; }

 protected:
  EGLDisplay display_;
  bool sub_environment_;
  bool valid_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_H_
