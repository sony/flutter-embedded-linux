// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_drm.h"

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"

namespace flutter {

SurfaceGlDrm::SurfaceGlDrm(std::unique_ptr<ContextEglDrm> context)
    : native_window_(nullptr), onscreen_surface_(nullptr) {
  context_ = std::move(context);
}

bool SurfaceGlDrm::IsValid() const {
  return offscreen_surface_ && context_->IsValid();
}

bool SurfaceGlDrm::SetNativeWindow(NativeWindow<gbm_surface>* window) {
  native_window_ = window;
  onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
  if (!onscreen_surface_->IsValid()) {
    return false;
  }
  return true;
}

bool SurfaceGlDrm::SetNativeWindowResource(NativeWindow<gbm_surface>* window) {
  offscreen_surface_ = context_->CreateOffscreenSurface(window);
  if (!offscreen_surface_->IsValid()) {
    LINUXES_LOG(WARNING) << "Off-Screen surface is invalid.";
    offscreen_surface_ = nullptr;
    return false;
  }
  return true;
}

bool SurfaceGlDrm::OnScreenSurfaceResize(const size_t width,
                                         const size_t height) const {
  return native_window_->Resize(width, height);
}

void SurfaceGlDrm::DestroyOnScreenContext() {
  context_->ClearCurrent();
  onscreen_surface_ = nullptr;
}

bool SurfaceGlDrm::ResourceContextMakeCurrent() const {
  if (!offscreen_surface_) {
    return false;
  }
  return offscreen_surface_->MakeCurrent();
}

bool SurfaceGlDrm::ClearCurrentContext() const {
  return context_->ClearCurrent();
}

bool SurfaceGlDrm::GLContextMakeCurrent() const {
  return onscreen_surface_->MakeCurrent();
}

bool SurfaceGlDrm::GLContextClearCurrent() const {
  return context_->ClearCurrent();
}

bool SurfaceGlDrm::GLContextPresent(uint32_t fbo_id) const {
  if (!onscreen_surface_->SwapBuffers()) {
    return false;
  }
  static_cast<NativeWindowDrm*>(native_window_)->SwapBuffer();
  return true;
}

uint32_t SurfaceGlDrm::GLContextFBO() const { return 0; }

void* SurfaceGlDrm::GlProcResolver(const char* name) const {
  return context_->GlProcResolver(name);
}

}  // namespace flutter
