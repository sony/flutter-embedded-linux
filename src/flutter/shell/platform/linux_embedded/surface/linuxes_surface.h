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
  bool IsValid() const { return offscreen_surface_ && context_->IsValid(); };

  // Sets a netive platform's window.
  bool SetNativeWindow(NativeWindow* window) {
    native_window_ = window;
    onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
    if (!onscreen_surface_->IsValid()) {
      return false;
    }
    return true;
  };

  // Sets a netive platform's window for offscreen.
  bool SetNativeWindowResource(NativeWindow* window) {
    offscreen_surface_ = context_->CreateOffscreenSurface(window);
    if (!offscreen_surface_->IsValid()) {
      LINUXES_LOG(WARNING) << "Off-Screen surface is invalid.";
      offscreen_surface_ = nullptr;
      return false;
    }
    return true;
  }

  bool SetNativeWindowResource(std::unique_ptr<NativeWindow> window) {
    native_window_resource_ = std::move(window);
    return SetNativeWindowResource(native_window_resource_.get());
  }

  // Changes an on-screen surface size.
  bool OnScreenSurfaceResize(const size_t width, const size_t height) const {
    return native_window_->Resize(width, height);
  };

  // Clears current on-screen context.
  bool ClearCurrentContext() const { return context_->ClearCurrent(); };

  // Clears and destroys current ons-screen context.
  void DestroyOnScreenContext() {
    context_->ClearCurrent();
    onscreen_surface_ = nullptr;
  };

  // Makes an off-screen resource context.
  bool ResourceContextMakeCurrent() const {
    if (!offscreen_surface_) {
      return false;
    }
    return offscreen_surface_->MakeCurrent();
  };

 protected:
  std::unique_ptr<ContextEgl> context_;
  NativeWindow* native_window_ = nullptr;
  std::unique_ptr<NativeWindow> native_window_resource_ = nullptr;
  std::unique_ptr<LinuxesEGLSurface> onscreen_surface_ = nullptr;
  std::unique_ptr<LinuxesEGLSurface> offscreen_surface_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_H_