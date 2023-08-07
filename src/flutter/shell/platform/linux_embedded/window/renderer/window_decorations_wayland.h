// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATIONS_WAYLAND_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATIONS_WAYLAND_H_

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#endif

#include <wayland-client.h>

#include <memory>
#include <vector>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/surface_decoration.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_wayland_decoration.h"
#include "flutter/shell/platform/linux_embedded/window/renderer/window_decoration_button.h"
#include "flutter/shell/platform/linux_embedded/window/renderer/window_decoration_titlebar.h"

namespace flutter {

class WindowDecorationsWayland {
 public:
  // @param[in] width_dip   Logical width of the window (i.e. surface width).
  // @param[in] height_dip  Logical height of the window (i.e. surface height).
  // @param[in] pixel_ratio Physical / logical pixels ratio.
  WindowDecorationsWayland(wl_display* display,
                           wl_compositor* compositor,
                           wl_subcompositor* subcompositor,
                           wl_surface* root_surface,
                           int32_t width_dip,
                           int32_t height_dip,
                           double pixel_ratio,
                           bool enable_impeller,
                           bool enable_vsync);
  ~WindowDecorationsWayland();

  void Draw();

  // @param[in] width_dip   Logical width of the window (i.e. surface width).
  // @param[in] height_dip  Logical height of the window (i.e. surface height).
  // @param[in] pixel_ratio Physical / logical pixels ratio.
  void Resize(const int32_t width_dip,
              const int32_t height_dip,
              double pixel_ratio);

  bool IsMatched(wl_surface* surface,
                 WindowDecoration::DecorationType decoration_type) const;

  // Get height in logical pixels.
  int32_t Height() const;

 private:
  void DestroyContext();

  std::unique_ptr<WindowDecorationTitlebar> titlebar_;
  std::vector<std::unique_ptr<WindowDecorationButton>> buttons_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATIONS_WAYLAND_H_
