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
    CLOSE_BUTTON = 0,
    MAXIMISE_BUTTON,
    MINIMISE_BUTTON,
    TITLE_BAR,
  };

  WindowDecoration() = default;
  virtual ~WindowDecoration() = default;

  virtual void Draw() = 0;

  // @param[in] x_dip          The x coordinate in logical pixels.
  // @param[in] y_dip          The y coordinate in logical pixels.
  virtual void SetPosition(const int32_t x_dip, const int32_t y_dip) = 0;

  // @param[in] width_px   Physical width of the window.
  // @param[in] height_px  Physical height of the window.
  virtual void Resize(const size_t width_px, const size_t height_px) = 0;

  // Sets the scale factor for the next commit. Scale factor persists until a
  // new one is set.
  virtual void SetScaleFactor(float scale_factor) = 0;

  void DestroyContext() const { render_surface_->DestroyContext(); };

  wl_surface* Surface() const { return native_window_->Surface(); };

  DecorationType Type() const { return decoration_type_; };

 protected:
  std::unique_ptr<NativeWindowWaylandDecoration> native_window_;
  std::unique_ptr<SurfaceDecoration> render_surface_;
  DecorationType decoration_type_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATION_H_
