// Copyright 2023 Sony Corporation. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugins/settings_plugin.h"

#include "flutter/shell/platform/common/json_message_codec.h"

namespace flutter {

namespace {
constexpr char kChannelName[] = "flutter/settings";

constexpr char kAlwaysUse24HourFormat[] = "alwaysUse24HourFormat";
constexpr char kTextScaleFactor[] = "textScaleFactor";
constexpr char kPlatformBrightness[] = "platformBrightness";

constexpr char kPlatformBrightnessDark[] = "dark";
constexpr char kPlatformBrightnessLight[] = "light";
}  // namespace

SettingsPlugin::SettingsPlugin(BinaryMessenger* messenger,
                               WindowBindingHandler* delegate)
    : channel_(std::make_unique<BasicMessageChannel<rapidjson::Document>>(
          messenger,
          kChannelName,
          &JsonMessageCodec::GetInstance())),
      delegate_(delegate) {}

void SettingsPlugin::SendSettings() {
  rapidjson::Document settings(rapidjson::kObjectType);
  auto& allocator = settings.GetAllocator();
  settings.AddMember(kAlwaysUse24HourFormat, GetAlwaysUse24HourFormat(),
                     allocator);
  settings.AddMember(kTextScaleFactor, GetTextScaleFactor(), allocator);

  if (GetPreferredBrightness() == PlatformBrightness::kDark) {
    settings.AddMember(kPlatformBrightness, kPlatformBrightnessDark, allocator);
  } else {
    settings.AddMember(kPlatformBrightness, kPlatformBrightnessLight,
                       allocator);
  }
  channel_->Send(settings);
}

bool SettingsPlugin::GetAlwaysUse24HourFormat() {
  // The current OS does not have 24 hour format factor.
  return true;
}

float SettingsPlugin::GetTextScaleFactor() {
  // The current OS does not have text scale factor.
  return 1.0;
}

SettingsPlugin::PlatformBrightness SettingsPlugin::GetPreferredBrightness() {
  // The current OS does not have brightness factor.
  return PlatformBrightness::kLight;
}

void SettingsPlugin::UpdateHighContrastMode(bool is_high_contrast) {
  is_high_contrast_ = is_high_contrast;
  SendSettings();
}

}  // namespace flutter
