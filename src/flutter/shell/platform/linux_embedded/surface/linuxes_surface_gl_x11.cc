// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_x11.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

SurfaceGlX11::SurfaceGlX11(std::unique_ptr<ContextEglX11> context)
    : native_window_(nullptr),
      onscreen_surface_(nullptr),
      offscreen_surface_(nullptr) {
  context_ = std::move(context);
}

bool SurfaceGlX11::IsValid() const {
  return offscreen_surface_ && context_->IsValid();
}

bool SurfaceGlX11::SetNativeWindow(NativeWindow<xcb_window_t>* window) {
  native_window_ = window;

  onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
  if (!onscreen_surface_->IsValid()) {
    return false;
  }

  offscreen_surface_ = context_->CreateOffscreenSurface(nullptr);
  if (!offscreen_surface_->IsValid()) {
    LINUXES_LOG(WARNING) << "Off-Screen surface is invalid.";
    offscreen_surface_ = nullptr;
    return false;
  }

  return true;
}

bool SurfaceGlX11::OnScreenSurfaceResize(const size_t width,
                                         const size_t height) const {
  return native_window_->Resize(width, height);
}

void SurfaceGlX11::DestroyOnScreenContext() {
  context_->ClearCurrent();
  onscreen_surface_ = nullptr;
}

bool SurfaceGlX11::ResourceContextMakeCurrent() const {
  return offscreen_surface_->MakeCurrent();
}

bool SurfaceGlX11::ClearCurrentContext() const {
  return context_->ClearCurrent();
}

bool SurfaceGlX11::GLContextMakeCurrent() const {
  return onscreen_surface_->MakeCurrent();
}

bool SurfaceGlX11::GLContextClearCurrent() const {
  return context_->ClearCurrent();
}

bool SurfaceGlX11::GLContextPresent(uint32_t fbo_id) const {
  return onscreen_surface_->SwapBuffers();
}

uint32_t SurfaceGlX11::GLContextFBO() const { return 0; }

void* SurfaceGlX11::GlProcResolver(const char* name) const {
  return context_->GlProcResolver(name);
}

}  // namespace flutter
