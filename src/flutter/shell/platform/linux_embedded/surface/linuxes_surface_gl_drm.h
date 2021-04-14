// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_H_

#include <memory>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_delegate.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"

namespace flutter {

template <typename C>
class SurfaceGlDrm final : public Surface, public SurfaceGlDelegate {
 public:
  SurfaceGlDrm(std::unique_ptr<C> context)
      : native_window_(nullptr), onscreen_surface_(nullptr) {
    context_ = std::move(context);
  }

  ~SurfaceGlDrm() = default;

  // |Surface|
  bool IsValid() const override {
    return offscreen_surface_ && context_->IsValid();
  }

  // |Surface|
  bool SetNativeWindow(NativeWindow* window) override {
    native_window_ = window;
    onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
    if (!onscreen_surface_->IsValid()) {
      return false;
    }
    return true;
  }

  bool SetNativeWindowResource(NativeWindow* window) {
    offscreen_surface_ = context_->CreateOffscreenSurface(window);
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
    if (!offscreen_surface_) {
      return false;
    }
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
    if (!onscreen_surface_->SwapBuffers()) {
      return false;
    }
    static_cast<NativeWindowDrm<SurfaceGlDrm<C>>*>(native_window_)
        ->SwapBuffer();
    return true;
  }

  // |SurfaceGlDelegate|
  uint32_t GLContextFBO() const override { return 0; }

  // |SurfaceGlDelegate|
  void* GlProcResolver(const char* name) const override {
    return context_->GlProcResolver(name);
  }

 protected:
  std::unique_ptr<C> context_;
  NativeWindow* native_window_;
  std::unique_ptr<LinuxesEGLSurface> onscreen_surface_;
  std::unique_ptr<LinuxesEGLSurface> offscreen_surface_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_H_