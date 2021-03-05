// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_NATIVE_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_NATIVE_WINDOW_H_

namespace flutter {

template <typename W, typename S>
class NativeWindow {
 public:
  NativeWindow() = default;
  virtual ~NativeWindow() = default;

  bool IsValid() const { return valid_; };

  W* Window() const { return window_; }

  S* Surface() const { return surface_; }

  virtual bool Resize(const size_t width, const size_t height) const = 0;

 protected:
  W* window_ = nullptr;
  S* surface_ = nullptr;
  bool valid_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_NATIVE_WINDOW_H_