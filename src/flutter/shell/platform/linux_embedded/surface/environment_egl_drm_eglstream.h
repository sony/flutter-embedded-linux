// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_DRM_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_DRM_EGLSTREAM_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "flutter/shell/platform/linux_embedded/surface/environment_egl.h"

namespace flutter {

class EnvironmentEglDrmEglstream : public EnvironmentEgl {
 public:
  EnvironmentEglDrmEglstream();
  ~EnvironmentEglDrmEglstream() = default;

 private:
  bool SetEglExtensionFunctionPointers();

  EGLDeviceEXT GetEglDevice();

  PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT_;
  PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT_;
  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_DRM_EGLSTREAM_H_
