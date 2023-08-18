// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_H_

#include <cmath>

#include "flutter/shell/platform/linux_embedded/public/flutter_elinux.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"

namespace flutter {

class ELinuxWindow {
 public:
  ELinuxWindow() = default;
  virtual ~ELinuxWindow() = default;

 protected:
  virtual bool IsValid() const = 0;

  // Get current window width in physical pixels.
  uint32_t GetCurrentWidth() const {
    return view_properties_.width * current_scale_;
  }

  // Get current window height in physical pixels.
  uint32_t GetCurrentHeight() const {
    return view_properties_.height * current_scale_;
  }

  void SetRotation(FlutterDesktopViewRotation rotation) {
    if (rotation == FlutterDesktopViewRotation::kRotation_90) {
      current_rotation_ = 90;
    } else if (rotation == FlutterDesktopViewRotation::kRotation_180) {
      current_rotation_ = 180;
    } else if (rotation == FlutterDesktopViewRotation::kRotation_270) {
      current_rotation_ = 270;
    } else {
      current_rotation_ = 0;
    }
  }

  void NotifyDisplayInfoUpdates() const {
    if (binding_handler_delegate_) {
      binding_handler_delegate_->UpdateDisplayInfo(
          std::trunc(1000000.0 / frame_rate_), GetCurrentWidth(),
          GetCurrentHeight(), current_scale_);
    }
  }

  FlutterDesktopViewProperties view_properties_;

  // A pointer to a FlutterWindowsView that can be used to update engine
  // windowing and input state.
  WindowBindingHandlerDelegate* binding_handler_delegate_ = nullptr;

  int32_t display_max_width_ = -1;
  int32_t display_max_height_ = -1;
  int32_t frame_rate_ = 60000;
  double current_scale_ = 1.0;
  uint16_t current_rotation_ = 0;
  // The x coordinate of the pointer in physical pixels.
  double pointer_x_ = 0;
  // The y coordinate of the pointer in physical pixels.
  double pointer_y_ = 0;
  std::string clipboard_data_ = "";
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_H_
