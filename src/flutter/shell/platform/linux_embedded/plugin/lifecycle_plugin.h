// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_LIFECYCLE_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_LIFECYCLE_PLUGIN_H_

#include <memory>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/basic_message_channel.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"

namespace flutter {

class LifecyclePlugin {
 public:
  LifecyclePlugin(BinaryMessenger* messenger);
  ~LifecyclePlugin() = default;

  void OnInactive() const;

  void OnResumed() const;

  void OnPaused() const;

  void OnDetached() const;

 private:
  std::unique_ptr<flutter::BasicMessageChannel<EncodableValue>> channel_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_LIFECYCLE_PLUGIN_H_
