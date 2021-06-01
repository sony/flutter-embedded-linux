// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_PLATFORM_VIEWS_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_PLATFORM_VIEWS_PLUGIN_H_

#include <memory>
#include <unordered_map>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/linux_embedded/public/flutter_platform_views.h"

namespace flutter {

class PlatformViewsPlugin {
 public:
  PlatformViewsPlugin(BinaryMessenger* messenger);
  ~PlatformViewsPlugin();

  // Registers a factory of the platform view.
  void RegisterViewFactory(
      const char* view_type,
      std::unique_ptr<FlutterDesktopPlatformViewFactory> factory);

 private:
  // Called when a method is called on |channel_|;
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Called when "create" method is called
  void PlatformViewsCreate(
      const flutter::EncodableValue& arguments,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Called when "dispose" method is called
  void PlatformViewsDispose(
      const flutter::EncodableValue& arguments,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Method channel instance.
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;

  // Factory of PlatformView class.
  std::unordered_map<std::string,
                     std::unique_ptr<FlutterDesktopPlatformViewFactory>>
      platform_view_factories_;

  // Platform views (instances).
  std::unordered_map<int, FlutterDesktopPlatformView*> platform_views_;

  // Shows the id of current view.
  int current_view_id_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_PLATFORM_VIEWS_PLUGIN_H_
