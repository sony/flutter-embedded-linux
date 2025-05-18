// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"

namespace flutter {

class EnvironmentEgl {
 public:
  EnvironmentEgl(EGLenum platform, EGLNativeDisplayType platform_display,
                 bool sub_environment = false)
      : display_(EGL_NO_DISPLAY), sub_environment_(sub_environment) {
    InitPlatformFuns();
    if (platform && eglGetPlatformDisplay_) {
        display_ = eglGetPlatformDisplay_(platform, platform_display, nullptr);
    }
    if (display_ == EGL_NO_DISPLAY) {
        display_ = eglGetDisplay(platform_display);
    }
    if (display_ == EGL_NO_DISPLAY) {
      ELINUX_LOG(ERROR) << "Failed to get the EGL display: "
                        << get_egl_error_cause();
      return;
    }

    // sub_environment flag is used for window decorations such as toolbar and
    // buttons. When this flag is active, EGLDisplay doesn't be initialized and
    // finalized.
    if (!sub_environment_) {
      valid_ = InitializeEgl();
    } else {
      valid_ = true;
    }
  }

  EnvironmentEgl(bool sub_environment = false)
      : display_(EGL_NO_DISPLAY), sub_environment_(sub_environment) {}

  ~EnvironmentEgl() {
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

 private:
  void InitPlatformFuns() {
    static const char* client_exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (client_exts && has_egl_extension(client_exts, "EGL_EXT_platform_base")) {
        eglGetPlatformDisplay_ = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    }
  }

 protected:
  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplay_ = nullptr;
  EGLDisplay display_;
  bool valid_ = false;
  bool sub_environment_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_H_
