// Copyright 2023 Sony Corporation. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_SETTINGS_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_SETTINGS_PLUGIN_H_

#include <rapidjson/document.h>

#include <memory>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/basic_message_channel.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"

namespace flutter {

// Abstract settings plugin.
//
// Used to look up and notify Flutter of user-configured system settings.
// These are typically set in the control panel.
class SettingsPlugin {
 public:
  enum struct PlatformBrightness { kDark, kLight };

  explicit SettingsPlugin(BinaryMessenger* messenger,
                          WindowBindingHandler* delegate);
  ~SettingsPlugin() = default;

  // Sends settings (e.g., platform brightness) to the engine.
  void SendSettings();

  // Update the always use 24hour-format status of the system.
  void UpdateAlwaysUse24HourFormat(bool is_always_use_24hour_format);

  // Update the high contrast status of the system.
  void UpdateHighContrastMode(bool is_high_contrast);

  // Update the text scale factor of the system.
  void UpdateTextScaleFactor(float factor);

 private:
  // Returns `true` if the user uses 24 hour time.
  bool GetAlwaysUse24HourFormat();

  // Returns the user-preferred text scale factor.
  float GetTextScaleFactor();

  // Returns the user-preferred brightness.
  PlatformBrightness GetPreferredBrightness();

  std::unique_ptr<BasicMessageChannel<rapidjson::Document>> channel_;
  WindowBindingHandler* delegate_;
  bool is_high_contrast_ = false;
  bool is_always_use_24hour_format_ = true;
  float text_scaling_factor_ = 1.0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_SETTINGS_PLUGIN_H_
