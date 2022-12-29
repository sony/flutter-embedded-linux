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

  // Get physical width of the window.
  int32_t Width() const {
    if (!valid_) {
      return -1;
    }
    return width_;
  }

  // Get physical height of the window.
  int32_t Height() const {
    if (!valid_) {
      return -1;
    }
    return height_;
  }

  virtual bool IsNeedRecreateSurfaceAfterResize() const { return false; }

  // Sets a window position. Basically, this API is used for window decorations
  // such as titlebar.
  // @param[in] x_dip   The x coordinate in logical pixels.
  // @param[in] y_dip   The y coordinate in logical pixels.
  virtual void SetPosition(const int32_t x_dip, const int32_t y_dip) {
    x_ = x_dip;
    y_ = y_dip;
  };

  // @param[in] width_px       Physical width of the window.
  // @param[in] height_px      Physical height of the window.
  virtual bool Resize(const size_t width_px, const size_t height_px) = 0;

  // Swaps frame buffers. This API performs processing only for the DRM-GBM
  // backend. It is prepared to make the interface common.
  virtual void SwapBuffers(){/* do nothing. */};

 protected:
  EGLNativeWindowType window_;
  EGLNativeWindowType window_offscreen_;
  // Physical width of the window.
  int32_t width_;
  // Physical height of the window.
  int32_t height_;
  // The x coordinate of the window in logical pixels.
  int32_t x_;
  // The y coordinate of the window in logical pixels.
  int32_t y_;
  bool valid_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_H_
