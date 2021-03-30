// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_eglstream.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

SurfaceGlEglstream::SurfaceGlEglstream(
    std::unique_ptr<ContextEglEglstream> context)
    : native_window_(nullptr), onscreen_surface_(nullptr) {
  context_ = std::move(context);
}

bool SurfaceGlEglstream::IsValid() const {
  return offscreen_surface_ && context_->IsValid();
}

bool SurfaceGlEglstream::SetNativeWindow(NativeWindow<void>* window) {
  native_window_ = window;
  onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
  if (!onscreen_surface_->IsValid()) {
    return false;
  }
  return true;
}

bool SurfaceGlEglstream::SetNativeWindowResource(NativeWindow<void>* window) {
  offscreen_surface_ = context_->CreateOffscreenSurface(window);
  if (!offscreen_surface_->IsValid()) {
    LINUXES_LOG(WARNING) << "Off-Screen surface is invalid.";
    offscreen_surface_ = nullptr;
    return false;
  }
  return true;
}

bool SurfaceGlEglstream::OnScreenSurfaceResize(const size_t width,
                                               const size_t height) const {
  return native_window_->Resize(width, height);
}

void SurfaceGlEglstream::DestroyOnScreenContext() {
  context_->ClearCurrent();
  onscreen_surface_ = nullptr;
}

bool SurfaceGlEglstream::ResourceContextMakeCurrent() const {
  if (!offscreen_surface_) {
    return false;
  }
  return offscreen_surface_->MakeCurrent();
}

bool SurfaceGlEglstream::ClearCurrentContext() const {
  return context_->ClearCurrent();
}

bool SurfaceGlEglstream::GLContextMakeCurrent() const {
  return onscreen_surface_->MakeCurrent();
}

bool SurfaceGlEglstream::GLContextClearCurrent() const {
  return context_->ClearCurrent();
}

bool SurfaceGlEglstream::GLContextPresent(uint32_t fbo_id) const {
  return onscreen_surface_->SwapBuffers();
}

uint32_t SurfaceGlEglstream::GLContextFBO() const { return 0; }

void* SurfaceGlEglstream::GlProcResolver(const char* name) const {
  return context_->GlProcResolver(name);
}

}  // namespace flutter
