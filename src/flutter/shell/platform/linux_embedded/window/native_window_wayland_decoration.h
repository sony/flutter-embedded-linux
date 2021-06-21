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
  NativeWindowWaylandDecoration(wl_compositor* compositor,
                                wl_subcompositor* subcompositor,
                                wl_surface* parent_surface, const size_t width,
                                const size_t height);
  ~NativeWindowWaylandDecoration();

  // |NativeWindow|
  bool Resize(const size_t width, const size_t height) override;

  wl_surface* Surface() const { return surface_; }

 private:
  wl_surface* surface_ = nullptr;
  wl_subsurface* subsurface_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_WAYLAND_DECORATION_H_
