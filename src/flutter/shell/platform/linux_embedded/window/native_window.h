// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_H_

namespace flutter {

template <typename T>
class NativeWindow {
 public:
  NativeWindow() = default;
  virtual ~NativeWindow() = default;

  bool IsValid() const { return valid_; };

#if defined(DISPLAY_BACKEND_TYPE_X11) || \
    defined(DISPLAY_BACKEND_TYPE_DRM_EGLSTREAM)
  T Window() const { return window_; }
#else
  T* Window() const { return window_; }
#endif

  int32_t Width() {
    if (!valid_) {
      return -1;
    }
    return width_;
  }

  int32_t Height() {
    if (!valid_) {
      return -1;
    }
    return height_;
  }

  virtual bool Resize(const size_t width, const size_t height) = 0;

 protected:
  // Specifies the native winodw (NativeWindowType)
#if defined(DISPLAY_BACKEND_TYPE_X11) || \
    defined(DISPLAY_BACKEND_TYPE_DRM_EGLSTREAM)
  T window_;
#else
  T* window_ = nullptr;
#endif

  bool valid_ = false;
  int32_t width_;
  int32_t height_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_H_