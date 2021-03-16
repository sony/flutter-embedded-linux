// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_wayland.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

SurfaceGlWayland::SurfaceGlWayland(std::unique_ptr<ContextEglWayland> context)
    : native_window_(nullptr),
      onscreen_surface_(nullptr),
      offscreen_surface_(nullptr) {
  context_ = std::move(context);
}

bool SurfaceGlWayland::IsValid() const override {
  return offscreen_surface_ && context_->IsValid();
}

bool SurfaceGlWayland::SetNativeWindow(
    NativeWindow<wl_egl_window, wl_surface>* window) override {
  native_window_ = window;
  onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
  if (!onscreen_surface_->IsValid()) {
    return false;
  }
  return true;
}

bool SurfaceGlWayland::SetNativeWindowResource(
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
bool SurfaceGlWayland::OnScreenSurfaceResize(
    const size_t width, const size_t height) const override {
  return native_window_->Resize(width, height);
}

// |Surface|
void SurfaceGlWayland::DestroyOnScreenContext() override {
  context_->ClearCurrent();
  onscreen_surface_ = nullptr;
}

// |Surface|
bool SurfaceGlWayland::ResourceContextMakeCurrent() const override {
  return offscreen_surface_->MakeCurrent();
}

// |Surface|
bool SurfaceGlWayland::ClearCurrentContext() const override {
  return context_->ClearCurrent();
}

// |SurfaceGlDelegate|
bool SurfaceGlWayland::GLContextMakeCurrent() const override {
  return onscreen_surface_->MakeCurrent();
}

// |SurfaceGlDelegate|
bool SurfaceGlWayland::GLContextClearCurrent() const override {
  return context_->ClearCurrent();
}

// |SurfaceGlDelegate|
bool SurfaceGlWayland::GLContextPresent(uint32_t fbo_id) const override {
  return onscreen_surface_->SwapBuffers();
}

// |SurfaceGlDelegate|
uint32_t SurfaceGlWayland::GLContextFBO() const override { return 0; }

// |SurfaceGlDelegate|
void* SurfaceGlWayland::GlProcResolver(const char* name) const override {
  return context_->GlProcResolver(name);
}

}  // namespace flutter
