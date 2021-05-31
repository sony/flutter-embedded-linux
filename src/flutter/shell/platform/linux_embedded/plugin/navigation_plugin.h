// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_NAVIGATION_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_NAVIGATION_PLUGIN_H_

#include <rapidjson/document.h>

#include <memory>
#include <string>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_channel.h"

namespace flutter {

class NavigationPlugin {
 public:
  NavigationPlugin(BinaryMessenger* messenger);
  ~NavigationPlugin() = default;

  void SetInitialRoute(std::string initial_route) const;

  void PushRoute(std::string route) const;

  void PopRoute() const;

 private:
  std::unique_ptr<flutter::MethodChannel<rapidjson::Document>> channel_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_NAVIGATION_PLUGIN_H_
