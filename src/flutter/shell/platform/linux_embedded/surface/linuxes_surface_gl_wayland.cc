// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_wayland.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

SurfaceGlWayland::SurfaceGlWayland(std::unique_ptr<ContextEgl> context)
    : native_window_(nullptr),
      onscreen_surface_(nullptr),
      offscreen_surface_(nullptr) {
  context_ = std::move(context);
}

bool SurfaceGlWayland::IsValid() const {
  return offscreen_surface_ && context_->IsValid();
}

bool SurfaceGlWayland::SetNativeWindow(NativeWindow* window) {
  native_window_ = window;
  onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
  if (!onscreen_surface_->IsValid()) {
    return false;
  }
  return true;
}

bool SurfaceGlWayland::SetNativeWindowResource(
    std::unique_ptr<NativeWindow> window) {
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

bool SurfaceGlWayland::OnScreenSurfaceResize(const size_t width,
                                             const size_t height) const {
  return native_window_->Resize(width, height);
}

void SurfaceGlWayland::DestroyOnScreenContext() {
  context_->ClearCurrent();
  onscreen_surface_ = nullptr;
}

bool SurfaceGlWayland::ResourceContextMakeCurrent() const {
  return offscreen_surface_->MakeCurrent();
}

bool SurfaceGlWayland::ClearCurrentContext() const {
  return context_->ClearCurrent();
}

bool SurfaceGlWayland::GLContextMakeCurrent() const {
  return onscreen_surface_->MakeCurrent();
}

bool SurfaceGlWayland::GLContextClearCurrent() const {
  return context_->ClearCurrent();
}

bool SurfaceGlWayland::GLContextPresent(uint32_t fbo_id) const {
  return onscreen_surface_->SwapBuffers();
}

uint32_t SurfaceGlWayland::GLContextFBO() const { return 0; }

void* SurfaceGlWayland::GlProcResolver(const char* name) const {
  return context_->GlProcResolver(name);
}

}  // namespace flutter
