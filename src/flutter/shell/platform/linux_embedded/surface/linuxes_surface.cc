// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

bool Surface::IsValid() const {
  return offscreen_surface_ && context_->IsValid();
};

bool Surface::SetNativeWindow(NativeWindow* window) {
  native_window_ = window;
  onscreen_surface_ = context_->CreateOnscreenSurface(native_window_);
  if (!onscreen_surface_->IsValid()) {
    return false;
  }
  return true;
};

bool Surface::SetNativeWindowResource(NativeWindow* window) {
  offscreen_surface_ = context_->CreateOffscreenSurface(window);
  if (!offscreen_surface_->IsValid()) {
    LINUXES_LOG(WARNING) << "Off-Screen surface is invalid.";
    offscreen_surface_ = nullptr;
    return false;
  }
  return true;
}

bool Surface::SetNativeWindowResource(std::unique_ptr<NativeWindow> window) {
  native_window_resource_ = std::move(window);
  return SetNativeWindowResource(native_window_resource_.get());
}

bool Surface::OnScreenSurfaceResize(const size_t width, const size_t height) {
  if (!native_window_->Resize(width, height)) {
    return false;
  }

  // This API returns true only for the DRM-GBM backend.
  // On the DRM-GBM backend, the gbm-surface is recreated by notification of
  // Resize(). In this case, we also need to recreate the on/off-screen surface
  // with the newly created gbm-surface.
  if (native_window_->IsNeedRecreateSurface()) {
    DestroyOnScreenContext();
    SetNativeWindow(native_window_);
    SetNativeWindowResource(native_window_);
  }
  return true;
};

bool Surface::ClearCurrentContext() const { return context_->ClearCurrent(); };

void Surface::DestroyOnScreenContext() {
  context_->ClearCurrent();
  onscreen_surface_ = nullptr;
};

bool Surface::ResourceContextMakeCurrent() const {
  if (!offscreen_surface_) {
    return false;
  }
  return offscreen_surface_->MakeCurrent();
};

}  // namespace flutter
