// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_X11_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_X11_H_

#include <X11/Xlib.h>
#include <xcb/xcb.h>

#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

class NativeWindowX11 : public NativeWindow<xcb_window_t> {
 public:
  NativeWindowX11(Display* display, const size_t width, const size_t height);
  ~NativeWindowX11();

  // |NativeWindow|
  bool Resize(const size_t width, const size_t height) override;

  xcb_connection_t* XcbConnection() { return xcb_connection_; }

  xcb_intern_atom_reply_t* WmDeleteMessage() { return reply_delete_; }

  void Destroy();

 private:
  xcb_connection_t* xcb_connection_ = nullptr;
  xcb_intern_atom_reply_t* reply_delete_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_X11_H_