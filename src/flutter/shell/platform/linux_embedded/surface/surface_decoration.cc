// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/surface_decoration.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

SurfaceDecoration::SurfaceDecoration(std::unique_ptr<ContextEgl> context) {
  context_ = std::move(context);
}

bool SurfaceDecoration::IsValid() const { return context_->IsValid(); };

bool SurfaceDecoration::SetNativeWindow(NativeWindow* window) {
  native_window_ = window;

  surface_ = context_->CreateOnscreenSurface(native_window_);
  if (!surface_->IsValid()) {
    return false;
  }

  return true;
};

bool SurfaceDecoration::Resize(const size_t width, const size_t height) {
  if (!native_window_->Resize(width, height)) {
    ELINUX_LOG(ERROR) << "Failed to resize.";
    return false;
  }

  if (native_window_->IsNeedRecreateSurfaceAfterResize()) {
    DestroyContext();
    surface_ = context_->CreateOnscreenSurface(native_window_);
    if (!surface_->IsValid()) {
      ELINUX_LOG(WARNING) << "Failed to recreate decoration surface.";
      surface_ = nullptr;
      return false;
    }
  }
  return true;
};

void SurfaceDecoration::DestroyContext() {
  context_->ClearCurrent();
  surface_ = nullptr;
};

bool SurfaceDecoration::GLContextMakeCurrent() const {
  return surface_->MakeCurrent();
}

bool SurfaceDecoration::GLContextClearCurrent() const {
  return context_->ClearCurrent();
}

bool SurfaceDecoration::GLContextPresent(uint32_t fbo_id) const {
  return surface_->SwapBuffers();
}

uint32_t SurfaceDecoration::GLContextFBO() const { return 0; }

void* SurfaceDecoration::GlProcResolver(const char* name) const {
  return context_->GlProcResolver(name);
}

}  // namespace flutter
