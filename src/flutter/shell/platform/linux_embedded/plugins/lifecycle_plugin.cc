// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugins/lifecycle_plugin.h"

#include "flutter/shell/platform/common/client_wrapper/include/flutter/string_message_codec.h"
#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
constexpr char kChannelName[] = "flutter/lifecycle";
constexpr char kInactive[] = "AppLifecycleState.inactive";
constexpr char kResumed[] = "AppLifecycleState.resumed";
constexpr char kPaused[] = "AppLifecycleState.paused";
constexpr char kDetached[] = "AppLifecycleState.detached";
constexpr char kHidden[] = "AppLifecycleState.hidden";

}  // namespace

LifecyclePlugin::LifecyclePlugin(BinaryMessenger* messenger)
    : channel_(std::make_unique<BasicMessageChannel<std::string>>(
          messenger,
          kChannelName,
          &StringMessageCodec::GetInstance())) {}

void LifecyclePlugin::OnInactive() const {
  ELINUX_LOG(DEBUG) << "App lifecycle changed to inactive state.";
  channel_->Send(std::string(kInactive));
}

void LifecyclePlugin::OnResumed() const {
  ELINUX_LOG(DEBUG) << "App lifecycle changed to resumed state.";
  channel_->Send(std::string(kResumed));
}

void LifecyclePlugin::OnPaused() const {
  ELINUX_LOG(DEBUG) << "App lifecycle changed to paused state.";
  channel_->Send(std::string(kPaused));
}

void LifecyclePlugin::OnDetached() const {
  ELINUX_LOG(DEBUG) << "App lifecycle changed to detached state.";
  channel_->Send(std::string(kDetached));
}

void LifecyclePlugin::OnHidden() const {
  ELINUX_LOG(DEBUG) << "App lifecycle changed to hidden state.";
  channel_->Send(std::string(kHidden));
}

}  // namespace flutter
