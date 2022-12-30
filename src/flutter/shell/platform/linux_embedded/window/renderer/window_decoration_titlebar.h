// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATION_TITLEBAR_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATION_TITLEBAR_H_

#include "flutter/shell/platform/linux_embedded/window/renderer/window_decoration.h"

namespace flutter {

class WindowDecorationTitlebar : public WindowDecoration {
 public:
  WindowDecorationTitlebar(
      std::unique_ptr<NativeWindowWaylandDecoration> native_window,
      std::unique_ptr<SurfaceDecoration> render_surface);
  ~WindowDecorationTitlebar();

  void Draw() override;

  // |WindowDecoration|
  void SetPosition(const int32_t x_dip, const int32_t y_dip) override;

  // |WindowDecoration|
  void Resize(const size_t width_px, const size_t height_px) override;

  // |WindowDecoration|
  void SetScaleFactor(float scale_factor) override;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_WINDOW_DECORATION_TITLEBAR_H_
