// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_WAYLAND_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_WAYLAND_H_

#include <wayland-client.h>

#include <memory>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/context_egl_wayland.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_delegate.h"

namespace flutter {

class SurfaceGlWayland final : public Surface<wl_egl_window, wl_surface>,
                               public SurfaceGlDelegate {
 public:
  SurfaceGlWayland(std::unique_ptr<ContextEglWayland> context)
      : native_window_(nullptr),
        onscreen_surface_(nullptr),
        offscreen_surface_(nullptr) {
    context_ = std::move(context);
  }

  ~SurfaceGlWayland() = default;

  // |Surface|
  bool IsValid() const override {
    return offscreen_surface_ && context_->IsValid();
  }

  // |Surface|
  bool SetNativeWindow(
      NativeWindow<wl_egl_window, wl_surface>* window) override {
    native_window_ = window;
    onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
    if (!onscreen_surface_->IsValid()) {
      return false;
    }
    return true;
  }

  bool SetNativeWindowResource(
      std::unique_ptr<NativeWindow<wl_egl_window, wl_surface>> window) {
    native_window_resource_ = std::move(window);
    offscreen_surface_ =
        context_->CreateOffscreenSurface(native_window_resource_.get());
    if (!offscreen_surface_->IsValid()) {
      LINUXES_LOG(WARNING) << "Off-Screen surface is invalid.";
      offscreen_surface_ = nullptr;
      return false;
    }
    return true;
  }

  // |Surface|
  bool OnScreenSurfaceResize(const size_t width,
                             const size_t height) const override {
    return native_window_->Resize(width, height);
  }

  // |Surface|
  void DestroyOnScreenContext() override {
    context_->ClearCurrent();
    onscreen_surface_ = nullptr;
  }

  // |Surface|
  bool ResourceContextMakeCurrent() const override {
    return offscreen_surface_->MakeCurrent();
  }

  // |Surface|
  bool ClearCurrentContext() const override { return context_->ClearCurrent(); }

  // |SurfaceGlDelegate|
  bool GLContextMakeCurrent() const override {
    return onscreen_surface_->MakeCurrent();
  }

  // |SurfaceGlDelegate|
  bool GLContextClearCurrent() const override {
    return context_->ClearCurrent();
  }

  // |SurfaceGlDelegate|
  bool GLContextPresent(uint32_t fbo_id) const override {
    return onscreen_surface_->SwapBuffers();
  }

  // |SurfaceGlDelegate|
  uint32_t GLContextFBO() const override { return 0; }

  // |SurfaceGlDelegate|
  void* GlProcResolver(const char* name) const override {
    return context_->GlProcResolver(name);
  }

 private:
  std::unique_ptr<ContextEglWayland> context_;
  NativeWindow<wl_egl_window, wl_surface>* native_window_;
  std::unique_ptr<NativeWindow<wl_egl_window, wl_surface>>
      native_window_resource_;
  std::unique_ptr<LinuxesEGLSurface> onscreen_surface_;
  std::unique_ptr<LinuxesEGLSurface> offscreen_surface_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_WAYLAND_H_