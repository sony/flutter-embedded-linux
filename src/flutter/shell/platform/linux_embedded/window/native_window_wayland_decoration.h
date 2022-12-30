// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_WAYLAND_DECORATION_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_WAYLAND_DECORATION_H_

#include <wayland-client.h>
#include <wayland-egl.h>

#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

class NativeWindowWaylandDecoration : public NativeWindow {
 public:
  // @param[in] width_px       Physical width of the window.
  // @param[in] height_px      Physical height of the window.
  NativeWindowWaylandDecoration(wl_compositor* compositor,
                                wl_subcompositor* subcompositor,
                                wl_surface* parent_surface,
                                const size_t width_px,
                                const size_t height_px);
  ~NativeWindowWaylandDecoration();

  // |NativeWindow|
  bool Resize(const size_t width_px, const size_t height_px) override;

  // |NativeWindow|
  void SetPosition(const int32_t x_dip, const int32_t y_dip) override;

  wl_surface* Surface() const { return surface_; }

 private:
  wl_surface* surface_ = nullptr;
  wl_subsurface* subsurface_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_WAYLAND_DECORATION_H_
