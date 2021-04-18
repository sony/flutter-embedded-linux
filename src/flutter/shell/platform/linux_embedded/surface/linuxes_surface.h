// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_H_

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/linuxes_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

class Surface {
 public:
  // Shows a surface is valid or not.
  virtual bool IsValid() const = 0;

  // Sets a netive platform's window.
  virtual bool SetNativeWindow(NativeWindow* window) = 0;

  // Changes an on-screen surface size.
  virtual bool OnScreenSurfaceResize(const size_t width,
                                     const size_t height) const = 0;

  // Clears current on-screen context.
  virtual bool ClearCurrentContext() const = 0;

  // Clears and destroys current ons-screen context.
  virtual void DestroyOnScreenContext() = 0;

  // Makes an off-screen resource context.
  virtual bool ResourceContextMakeCurrent() const = 0;

 protected:
  NativeWindow* native_window_ = nullptr;
  std::unique_ptr<NativeWindow> native_window_resource_ = nullptr;
  std::unique_ptr<LinuxesEGLSurface> onscreen_surface_ = nullptr;
  std::unique_ptr<LinuxesEGLSurface> offscreen_surface_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_H_