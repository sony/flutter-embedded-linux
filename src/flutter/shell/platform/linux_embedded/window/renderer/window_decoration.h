// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATION_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATION_H_

#include <wayland-client.h>

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/surface_decoration.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_wayland_decoration.h"

namespace flutter {

class WindowDecoration {
 public:
  enum DecorationType {
    TITLE_BAR = 0,
    CLOSE_BUTTON,
    MINIMISE_BUTTON,
    MAXIMISE_BUTTON,
  };

  WindowDecoration() = default;
  virtual ~WindowDecoration() = default;

  virtual void Draw() = 0;

  virtual void SetPosition(const int32_t x, const int32_t y) = 0;

  virtual void Resize(const int32_t width, const int32_t height) = 0;

  wl_surface* Surface() const { return native_window_->Surface(); };

  DecorationType Type() const { return decoration_type_; };

 protected:
  std::unique_ptr<NativeWindowWaylandDecoration> native_window_;
  std::unique_ptr<SurfaceDecoration> render_surface_;
  DecorationType decoration_type_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATION_H_
