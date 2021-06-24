// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/surface_base.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

bool SurfaceBase::IsValid() const {
  return offscreen_surface_ && context_->IsValid();
};

bool SurfaceBase::SetNativeWindow(NativeWindow* window) {
  native_window_ = window;

  onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
  if (!onscreen_surface_->IsValid()) {
    return false;
  }

  offscreen_surface_ = context_->CreateOffscreenSurface(native_window_);
  if (!offscreen_surface_->IsValid()) {
    offscreen_surface_ = nullptr;
    return false;
  }

  return true;
};

bool SurfaceBase::OnScreenSurfaceResize(const size_t width,
                                        const size_t height) {
  if (!native_window_->Resize(width, height)) {
    ELINUX_LOG(ERROR) << "Failed to resize.";
    return false;
  }

  if (native_window_->IsNeedRecreateSurfaceAfterResize()) {
    DestroyOnScreenContext();
    onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
    if (!onscreen_surface_->IsValid()) {
      ELINUX_LOG(WARNING) << "Failed to recreate on-screen surface.";
      onscreen_surface_ = nullptr;
      return false;
    }
  }
  return true;
};

bool SurfaceBase::ClearCurrentContext() const {
  return context_->ClearCurrent();
};

void SurfaceBase::DestroyOnScreenContext() {
  context_->ClearCurrent();
  onscreen_surface_ = nullptr;
};

bool SurfaceBase::ResourceContextMakeCurrent() const {
  if (!offscreen_surface_) {
    return false;
  }
  return offscreen_surface_->MakeCurrent();
};

}  // namespace flutter
