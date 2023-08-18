// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_DELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_DELEGATE_H_

#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

class WindowBindingHandlerDelegate {
 public:
  // Notifies delegate that backing window size has changed. Typically called by
  // currently configured WindowBindingHandler
  // @param[in] width_px       Physical width of the window.
  // @param[in] height_px      Physical height of the window.
  virtual void OnWindowSizeChanged(size_t width_px, size_t height_px) const = 0;

  // Notifies delegate that backing window mouse has moved. Typically called by
  // currently configured WindowBindingHandler
  // @param[in] x_px The x coordinate of the pointer event in physical pixels.
  // @param[in] y_px The y coordinate of the pointer event in physical pixels.
  virtual void OnPointerMove(double x_px, double y_px) = 0;

  // Notifies delegate that backing window mouse pointer button has been
  // pressed. Typically called by currently configured WindowBindingHandler
  // @param[in] x_px The x coordinate of the pointer event in physical pixels.
  // @param[in] y_px The y coordinate of the pointer event in physical pixels.
  virtual void OnPointerDown(double x_px,
                             double y_px,
                             FlutterPointerMouseButtons button) = 0;

  // Notifies delegate that backing window mouse pointer button has been
  // released. Typically called by currently configured WindowBindingHandler
  // @param[in] x_px The x coordinate of the pointer event in physical pixels.
  // @param[in] y_px The y coordinate of the pointer event in physical pixels.
  virtual void OnPointerUp(double x_px,
                           double y_px,
                           FlutterPointerMouseButtons button) = 0;

  // Notifies delegate that backing window mouse pointer has left the window.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnPointerLeave() = 0;

  // Notifies delegate that backing window touch pointer has been pressed.
  // Typically called by currently configured WindowBindingHandler
  // @param[in]  time    Monotonically increasing timestamp in milliseconds.
  // @param[in]  id      The unique id of this touch point.
  // @param[in]  x       The Surface local x coordinate.
  // @param[in]  y       The Surface local y coordinate.
  virtual void OnTouchDown(uint32_t time, int32_t id, double x, double y) = 0;

  // Notifies delegate that backing window touch pointer has been released.
  // Typically called by currently configured WindowBindingHandler
  // @param[in]  time    Monotonically increasing timestamp in milliseconds.
  // @param[in]  id      The unique id of this touch point.
  virtual void OnTouchUp(uint32_t time, int32_t id) = 0;

  // Notifies delegate that backing window touch pointer has moved.
  // Typically called by currently configured WindowBindingHandler
  // @param[in]  time    Monotonically increasing timestamp in milliseconds.
  // @param[in]  id      The unique id of this touch point.
  // @param[in]  x       The Surface local x coordinate.
  // @param[in]  y       The Surface local y coordinate.
  virtual void OnTouchMotion(uint32_t time, int32_t id, double x, double y) = 0;

  // Notifies delegate that backing window touch pointer has been canceled.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnTouchCancel() = 0;

  // Notifies delegate that backing window key has been cofigured.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnKeyMap(uint32_t format, int fd, uint32_t size) = 0;

  // Notifies delegate that backing window key has been modifired.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnKeyModifiers(uint32_t mods_depressed,
                              uint32_t mods_latched,
                              uint32_t mods_locked,
                              uint32_t group) = 0;

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
  virtual void OnScroll(double x,
                        double y,
                        double delta_x,
                        double delta_y,
                        int scroll_offset_multiplier) = 0;

  // Notifies delegate that backing window vsync has happened.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnVsync(uint64_t last_frame_time_nanos,
                       uint64_t vsync_interval_time_nanos) = 0;

  // Update the status of the high contrast feature
  virtual void UpdateHighContrastEnabled(bool enabled) = 0;

  // Update the status of the text scaling factor feature
  virtual void UpdateTextScaleFactor(float factor) = 0;

  // Update the status of the corresponding to display changes.
  // @param[in] refresh_rate    Refresh rate of the display.
  // @param[in] width_px        Physical width of the display.
  // @param[in] height_px       Physical height of the display.
  // @param[in] pixel_ratio     Pixel ratio of the display.
  virtual void UpdateDisplayInfo(double refresh_rate,
                                 size_t width_px,
                                 size_t height_px,
                                 double pixel_ratio) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_DELEGATE_H_
