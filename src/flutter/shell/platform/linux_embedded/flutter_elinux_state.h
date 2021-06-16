// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_STATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_STATE_H_

#include <memory>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/incoming_message_dispatcher.h"
#include "flutter/shell/platform/embedder/embedder.h"

// Structs backing the opaque references used in the C API.
//
// DO NOT ADD ANY NEW CODE HERE. These are legacy, and are being phased out
// in favor of objects that own and manage the relevant functionality.

namespace flutter {
struct FlutterELinuxEngine;
struct FlutterELinuxView;
}  // namespace flutter

// Wrapper to distinguish the view controller ref from the view ref given out
// in the C API.
struct FlutterDesktopViewControllerState {
  // The view that backs this state object.
  std::unique_ptr<flutter::FlutterELinuxView> view;
};

// Wrapper to distinguish the plugin registrar ref from the engine ref given out
// in the C API.
struct FlutterDesktopPluginRegistrar {
  // The engine that owns this state object.
  flutter::FlutterELinuxEngine* engine = nullptr;
};

// Wrapper to distinguish the messenger ref from the engine ref given out
// in the C API.
struct FlutterDesktopMessenger {
  // The engine that owns this state object.
  flutter::FlutterELinuxEngine* engine = nullptr;
};

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_STATE_H_
