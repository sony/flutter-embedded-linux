// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_MOUSE_CURSOR_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_MOUSE_CURSOR_PLUGIN_H_

#include <memory>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/basic_message_channel.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"

namespace flutter {

class MouseCursorPlugin {
 public:
  using MouseCursorUpdate = std::function<void(const std::string& cursor_name)>;

  MouseCursorPlugin(BinaryMessenger* messenger, WindowBindingHandler* delegate);
  ~MouseCursorPlugin() = default;

 private:
  // Called when a method is called on |channel_|;
  void HandleMethodCall(
      const flutter::MethodCall<EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<EncodableValue>> result);

  // The MethodChannel used for communication with the Flutter engine.
  std::unique_ptr<flutter::MethodChannel<EncodableValue>> channel_;

  // The delegate for cursor updates.
  WindowBindingHandler* delegate_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_MOUSE_CURSOR_PLUGIN_H_