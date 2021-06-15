// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_H_

#include <EGL/egl.h>

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/elinux_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/environment_egl.h"
#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

class ContextEgl {
 public:
  ContextEgl(std::unique_ptr<EnvironmentEgl> environment,
             EGLint egl_surface_type = EGL_WINDOW_BIT);
  ~ContextEgl() = default;

  virtual std::unique_ptr<ELinuxEGLSurface> CreateOnscreenSurface(
      NativeWindow* window) const;

  std::unique_ptr<ELinuxEGLSurface> CreateOffscreenSurface(
      NativeWindow* window_resource) const;

  bool IsValid() const;

  bool ClearCurrent() const;

  void* GlProcResolver(const char* name) const;

  EGLint GetAttrib(EGLint attribute);

 protected:
  std::unique_ptr<EnvironmentEgl> environment_;
  EGLConfig config_;
  EGLContext context_;
  EGLContext resource_context_;
  bool valid_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_H_
