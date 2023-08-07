// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_wayland.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

NativeWindowWayland::NativeWindowWayland(wl_compositor* compositor,
                                         const size_t width_px,
                                         const size_t height_px,
                                         bool enable_vsync) {
  surface_ = wl_compositor_create_surface(compositor);
  if (!surface_) {
    ELINUX_LOG(ERROR) << "Failed to create the compositor surface.";
    return;
  }

  window_ = wl_egl_window_create(surface_, width_px, height_px);
  if (!window_) {
    ELINUX_LOG(ERROR) << "Failed to create the EGL window.";
    return;
  }

  // The offscreen (resource) surface will not be mapped, but needs to be a
  // wl_surface because ONLY window EGL surfaces are supported on Wayland.
  {
    surface_offscreen_ = wl_compositor_create_surface(compositor);
    if (!surface_offscreen_) {
      ELINUX_LOG(ERROR)
          << "Failed to create the compositor surface for off-screen.";
      return;
    }

    window_offscreen_ = wl_egl_window_create(surface_offscreen_, 1, 1);
    if (!window_offscreen_) {
      ELINUX_LOG(ERROR) << "Failed to create the EGL window for offscreen.";
      return;
    }
  }

  enable_vsync_ = enable_vsync;
  width_ = width_px;
  height_ = height_px;
  valid_ = true;
}

NativeWindowWayland::~NativeWindowWayland() {
  if (window_) {
    wl_egl_window_destroy(window_);
    window_ = nullptr;
  }

  if (window_offscreen_) {
    wl_egl_window_destroy(window_offscreen_);
    window_offscreen_ = nullptr;
  }

  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = nullptr;
  }

  if (surface_offscreen_) {
    wl_surface_destroy(surface_offscreen_);
    surface_offscreen_ = nullptr;
  }
}

bool NativeWindowWayland::Resize(const size_t width_px,
                                 const size_t height_px) {
  if (!valid_) {
    ELINUX_LOG(ERROR) << "Failed to resize the window.";
    return false;
  }
  wl_egl_window_resize(window_, width_px, height_px, 0, 0);

  width_ = width_px;
  height_ = height_px;
  return true;
}

}  // namespace flutter
