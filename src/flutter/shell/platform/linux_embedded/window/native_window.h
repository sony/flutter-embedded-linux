// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_H_

#include <EGL/egl.h>

namespace flutter {

class NativeWindow {
 public:
  NativeWindow() = default;
  virtual ~NativeWindow() = default;

  bool IsValid() const { return valid_; };

  EGLNativeWindowType Window() const { return window_; }

  // Gets a window (GBM surface) for offscreen resource.
  EGLNativeWindowType WindowOffscreen() const { return window_offscreen_; }

  int32_t Width() const {
    if (!valid_) {
      return -1;
    }
    return width_;
  }

  int32_t Height() const {
    if (!valid_) {
      return -1;
    }
    return height_;
  }

  virtual bool IsNeedRecreateSurfaceAfterResize() const { return false; }

  // Sets a window position. Basically, this API is used for window decorations
  // such as titlebar.
  virtual void SetPosition(const int32_t x, const int32_t y) {
    x_ = x;
    y_ = y;
  };

  virtual bool Resize(const size_t width, const size_t height) = 0;

  // Swaps frame buffers. This API performs processing only for the DRM-GBM
  // backend. It is prepared to make the interface common.
  virtual void SwapBuffers(){/* do nothing. */};

 protected:
  EGLNativeWindowType window_;
  EGLNativeWindowType window_offscreen_;
  int32_t width_;
  int32_t height_;
  int32_t x_;
  int32_t y_;
  bool valid_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_H_
