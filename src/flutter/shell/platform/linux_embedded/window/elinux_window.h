// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_H_

#include "flutter/shell/platform/linux_embedded/public/flutter_elinux.h"

namespace flutter {

class ELinuxWindow {
 public:
  ELinuxWindow() = default;
  virtual ~ELinuxWindow() = default;

 protected:
  virtual bool IsValid() const = 0;

  uint32_t GetCurrentWidth() const { return view_properties_.width; }

  uint32_t GetCurrentHeight() const { return view_properties_.height; }

  FlutterDesktopViewProperties view_properties_;
  double current_scale_ = 1.0;
  double pointer_x_ = 0;
  double pointer_y_ = 0;
  std::string clipboard_data_ = "";
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_H_
