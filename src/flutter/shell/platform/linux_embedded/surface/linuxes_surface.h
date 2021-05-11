// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_H_

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

class Surface {
 public:
  // Shows a surface is valid or not.
  bool IsValid() const;

  // Sets a netive platform's window.
  bool SetNativeWindow(NativeWindow* window);

  // Sets a netive platform's window for offscreen.
  bool SetNativeWindowResource(NativeWindow* window);

  // Sets a netive platform's window for offscreen.
  bool SetNativeWindowResource(std::unique_ptr<NativeWindow> window);

  // Changes an on-screen surface size.
  // In the case of the DRM-GBM backend, recreate the on-screen surface because
  // the gbm-surface is recreated.
  bool OnScreenSurfaceResize(const size_t width, const size_t height);

  // Clears current on-screen context.
  bool ClearCurrentContext() const;

  // Clears and destroys current ons-screen context.
  void DestroyOnScreenContext();

  // Makes an off-screen resource context.
  bool ResourceContextMakeCurrent() const;

 protected:
  std::unique_ptr<ContextEgl> context_;
  NativeWindow* native_window_ = nullptr;
  std::unique_ptr<NativeWindow> native_window_resource_ = nullptr;
  std::unique_ptr<LinuxesEGLSurface> onscreen_surface_ = nullptr;
  std::unique_ptr<LinuxesEGLSurface> offscreen_surface_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_H_
