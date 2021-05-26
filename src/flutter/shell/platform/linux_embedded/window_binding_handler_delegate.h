// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_DELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_DELEGATE_H_

#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

class WindowBindingHandlerDelegate {
 public:
  // Notifies delegate that backing window size has changed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnWindowSizeChanged(size_t width, size_t height) const = 0;

  // Notifies delegate that backing window mouse has moved.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnPointerMove(double x, double y) = 0;

  // Notifies delegate that backing window mouse pointer button has been
  // pressed. Typically called by currently configured WindowBindingHandler
  virtual void OnPointerDown(double x, double y,
                             FlutterPointerMouseButtons button) = 0;

  // Notifies delegate that backing window mouse pointer button has been
  // released. Typically called by currently configured WindowBindingHandler
  virtual void OnPointerUp(double x, double y,
                           FlutterPointerMouseButtons button) = 0;

  // Notifies delegate that backing window mouse pointer has left the window.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnPointerLeave() = 0;

  // Notifies delegate that backing window touch pointer has been pressed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnTouchDown(uint32_t time, int32_t id, double x, double y) = 0;

  // Notifies delegate that backing window touch pointer has been released.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnTouchUp(uint32_t time, int32_t id) = 0;

  // Notifies delegate that backing window touch pointer has moved.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnTouchMotion(uint32_t time, int32_t id, double x, double y) = 0;

  // Notifies delegate that backing window touch pointer has been canceled.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnTouchCancel() = 0;

  // Notifies delegate that backing window key has been cofigured.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnKeyMap(uint32_t format, int fd, uint32_t size) = 0;

  // Notifies delegate that backing window key has been modifired.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnKeyModifiers(uint32_t mods_depressed, uint32_t mods_latched,
                              uint32_t mods_locked, uint32_t group) = 0;

  // Notifies delegate that backing window key has been pressed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnKey(uint32_t key, bool pressed) = 0;

  // Notifies delegate that backing window virtual key has been pressed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnVirtualKey(uint32_t code_point) = 0;

  // Notifies delegate that backing window virtual special key has been pressed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnVirtualSpecialKey(uint32_t keycode) = 0;

  // Notifies delegate that backing window size has recevied scroll.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnScroll(double x, double y, double delta_x, double delta_y,
                        int scroll_offset_multiplier) = 0;

  // Notifies delegate that backing window vsync has happened.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnVsync(uint64_t last_frame_time_nanos,
                       uint64_t vsync_interval_time_nanos) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_DELEGATE_H_
