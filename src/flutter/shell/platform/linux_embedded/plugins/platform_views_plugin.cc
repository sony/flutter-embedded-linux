// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugins/platform_views_plugin.h"

#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"
#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
constexpr char kChannelName[] = "flutter/platform_views";

constexpr char kCreateMethod[] = "create";
constexpr char kDisposeMethod[] = "dispose";
constexpr char kResizeMethod[] = "resize";
constexpr char kTouchMethod[] = "touch";
constexpr char kSetDirectionMethod[] = "setDirection";
constexpr char kClearFocusMethod[] = "clearFocus";
}  // namespace

PlatformViewsPlugin::PlatformViewsPlugin(BinaryMessenger* messenger)
    : channel_(
          std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
              messenger, kChannelName,
              &flutter::StandardMethodCodec::GetInstance())) {
  channel_->SetMethodCallHandler(
      [this](const flutter::MethodCall<flutter::EncodableValue>& call,
             std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
                 result) { HandleMethodCall(call, std::move(result)); });
}

PlatformViewsPlugin::~PlatformViewsPlugin() {}

void PlatformViewsPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const auto& method = method_call.method_name();
  const auto& arguments = *method_call.arguments();

  LINUXES_LOG(DEBUG) << "Platform views: " << method << " is called.";
  if (method.compare(kCreateMethod) == 0) {
  } else if (method.compare(kDisposeMethod) == 0) {
  } else if (method.compare(kResizeMethod) == 0) {
  } else if (method.compare(kTouchMethod) == 0) {
  } else if (method.compare(kSetDirectionMethod) == 0) {
  } else if (method.compare(kClearFocusMethod) == 0) {
  } else if (method.compare(kClearFocusMethod) == 0) {
  } else {
    result->NotImplemented();
    return;
  }
  result->Success();
}

}  // namespace flutter
