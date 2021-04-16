// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_X11_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_X11_H_

#include <X11/Xlib.h>

#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

class NativeWindowX11 : public NativeWindow {
 public:
  NativeWindowX11(Display* display, VisualID visual_id, const size_t width,
                  const size_t height);
  ~NativeWindowX11() = default;

  // |NativeWindow|
  bool Resize(const size_t width, const size_t height) override;

  void Destroy(Display* display);

 private:
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_X11_H_