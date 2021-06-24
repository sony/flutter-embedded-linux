// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_SUB_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_SUB_H_

#include <EGL/egl.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"
#include "flutter/shell/platform/linux_embedded/surface/environment_egl.h"

namespace flutter {

// EnvironmentEglSub is used for window decorations such as toolbar and buttons.
// The difference between EnvironmentEgl and EnvironmentEglSub is that
// EnvironmentEglSub doesn't initialize and finalize processes.
class EnvironmentEglSub {
 public:
  EnvironmentEglSub(EGLNativeDisplayType platform_display)
      : public EnvironmentEgl {
    display_ = eglGetDisplay(platform_display);
    if (display_ == EGL_NO_DISPLAY) {
      ELINUX_LOG(ERROR) << "Failed to get the EGL display: "
                        << get_egl_error_cause();
      return;
    }

    // Skips to call eglTerminate.
    sub_environment_ = true;

    valid_ = true;
  }

  ~EnvironmentEglSub() = default;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_SUB_H_
