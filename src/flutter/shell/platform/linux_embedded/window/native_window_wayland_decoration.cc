// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_wayland_decoration.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

NativeWindowWaylandDecoration::NativeWindowWaylandDecoration(
    wl_compositor* compositor,
    wl_subcompositor* subcompositor,
    wl_surface* parent_surface,
    const size_t width_px,
    const size_t height_px) {
  surface_ = wl_compositor_create_surface(compositor);
  if (!surface_) {
    ELINUX_LOG(ERROR) << "Failed to create the compositor surface.";
    return;
  }

  subsurface_ =
      wl_subcompositor_get_subsurface(subcompositor, surface_, parent_surface);
  if (!subsurface_) {
    ELINUX_LOG(ERROR) << "Failed to get the subsurface.";
    return;
  }
  wl_subsurface_set_desync(subsurface_);
  wl_subsurface_set_position(subsurface_, 0, 0);

  window_ = wl_egl_window_create(surface_, 1, 1);
  if (!window_) {
    ELINUX_LOG(ERROR) << "Failed to create the EGL window.";
    return;
  }

  width_ = width_px;
  height_ = height_px;
  valid_ = true;
}

NativeWindowWaylandDecoration::~NativeWindowWaylandDecoration() {
  if (window_) {
    wl_egl_window_destroy(window_);
    window_ = nullptr;
  }

  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = nullptr;
  }
}

bool NativeWindowWaylandDecoration::Resize(const size_t width_px,
                                           const size_t height_px) {
  if (!valid_) {
    ELINUX_LOG(ERROR) << "Failed to resize the window.";
    return false;
  }

  width_ = width_px;
  height_ = height_px;
  wl_egl_window_resize(window_, width_px, height_px, 0, 0);
  return true;
}

void NativeWindowWaylandDecoration::SetPosition(const int32_t x_dip,
                                                const int32_t y_dip) {
  if (!valid_) {
    ELINUX_LOG(ERROR) << "Failed to set the position of the window.";
    return;
  }

  x_ = x_dip;
  y_ = y_dip;
  wl_subsurface_set_position(subsurface_, x_dip, y_dip);
}

void NativeWindowWaylandDecoration::SetScaleFactor(float scale_factor) {
  if (!valid_) {
    ELINUX_LOG(ERROR) << "Failed to set the scale factor of the window.";
    return;
  }

  wl_surface_set_buffer_scale(surface_, scale_factor);
}

}  // namespace flutter
