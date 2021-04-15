// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_wayland.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

NativeWindowWayland::NativeWindowWayland(wl_compositor* compositor,
                                         const size_t width,
                                         const size_t height) {
  surface_ = wl_compositor_create_surface(compositor);
  if (!surface_) {
    LINUXES_LOG(ERROR) << "Failed to create the compositor surface.";
    return;
  }

  window_ = wl_egl_window_create(surface_, width, height);
  if (!window_) {
    LINUXES_LOG(ERROR) << "Failed to create the EGL window.";
    return;
  }

  valid_ = true;
}

NativeWindowWayland::~NativeWindowWayland() {
  if (window_) {
    wl_egl_window_destroy(window_);
    window_ = nullptr;
  }

  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = nullptr;
  }
}

bool NativeWindowWayland::Resize(const size_t width,
                                 const size_t height) const {
  if (!valid_) {
    LINUXES_LOG(ERROR) << "Failed to resize the window.";
    return false;
  }
  wl_egl_window_resize(window_, width, height, 0, 0);
  return true;
}

}  // namespace flutter
