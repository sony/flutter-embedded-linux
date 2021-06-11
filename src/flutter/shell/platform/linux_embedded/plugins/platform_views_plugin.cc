// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugins/platform_views_plugin.h"

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
constexpr char kChannelName[] = "flutter/platform_views";

constexpr char kCreateMethod[] = "create";
constexpr char kDisposeMethod[] = "dispose";
constexpr char kResizeMethod[] = "resize";
constexpr char kSetDirectionMethod[] = "setDirection";
constexpr char kClearFocusMethod[] = "clearFocus";
constexpr char kTouchMethod[] = "touch";
constexpr char kAcceptGestureMethod[] = "acceptGesture";
constexpr char kRejectGestureMethod[] = "rejectGesture";
constexpr char kEnterMethod[] = "enter";
constexpr char kExitMethod[] = "exit";

constexpr char kViewTypeKey[] = "viewType";
constexpr char kIdKey[] = "id";
constexpr char kWidthKey[] = "width";
constexpr char kHeightKey[] = "height";
constexpr char kParamsKey[] = "params";
}  // namespace

PlatformViewsPlugin::PlatformViewsPlugin(BinaryMessenger* messenger)
    : channel_(
          std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
              messenger, kChannelName,
              &flutter::StandardMethodCodec::GetInstance())),
      current_view_id_(-1) {
  channel_->SetMethodCallHandler(
      [this](const flutter::MethodCall<flutter::EncodableValue>& call,
             std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
                 result) { HandleMethodCall(call, std::move(result)); });
}

PlatformViewsPlugin::~PlatformViewsPlugin() {
  for (auto& itr : platform_views_) {
    delete itr.second;
  }
  platform_views_.clear();

  for (auto& itr : platform_view_factories_) {
    itr.second->Dispose();
  }
  platform_view_factories_.clear();
}

void PlatformViewsPlugin::RegisterViewFactory(
    const char* view_type,
    std::unique_ptr<FlutterDesktopPlatformViewFactory> factory) {
  if (platform_view_factories_.find(view_type) !=
      platform_view_factories_.end()) {
    ELINUX_LOG(ERROR) << "Platform Views factory is already registered: "
                      << view_type;
    return;
  }
  platform_view_factories_[view_type] = std::move(factory);
}

void PlatformViewsPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const auto& method = method_call.method_name();
  const auto& arguments = *method_call.arguments();

  // todo: support all of the methods on the platform views.
  if (method.compare(kCreateMethod) == 0) {
    PlatformViewsCreate(arguments, std::move(result));
  } else if (method.compare(kDisposeMethod) == 0) {
    PlatformViewsDispose(arguments, std::move(result));
  } else if (method.compare(kResizeMethod) == 0) {
    result->NotImplemented();
  } else if (method.compare(kSetDirectionMethod) == 0) {
    result->NotImplemented();
  } else if (method.compare(kClearFocusMethod) == 0) {
    result->NotImplemented();
  } else if (method.compare(kTouchMethod) == 0) {
    result->NotImplemented();
  } else if (method.compare(kAcceptGestureMethod) == 0) {
    result->NotImplemented();
  } else if (method.compare(kRejectGestureMethod) == 0) {
    result->NotImplemented();
  } else if (method.compare(kEnterMethod) == 0) {
    result->NotImplemented();
  } else if (method.compare(kExitMethod) == 0) {
    result->NotImplemented();
  } else {
    ELINUX_LOG(WARNING) << "Platform Views unexpected method is called: "
                        << method;
    result->NotImplemented();
  }
}

void PlatformViewsPlugin::PlatformViewsCreate(
    const flutter::EncodableValue& arguments,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  auto view_type = LookupEncodableMap<std::string>(arguments, kViewTypeKey);
  if (view_type.empty()) {
    result->Error("Couldn't find the view type in the arguments");
    return;
  }
  auto view_id = LookupEncodableMap<int>(arguments, kIdKey);
  if (!view_id) {
    result->Error("Couldn't find the view id in the arguments");
    return;
  }
  auto view_width = LookupEncodableMap<double>(arguments, kWidthKey);
  if (!view_width) {
    result->Error("Couldn't find width in the arguments");
    return;
  }
  auto view_height = LookupEncodableMap<double>(arguments, kHeightKey);
  if (!view_height) {
    result->Error("Couldn't find height in the arguments");
    return;
  }
  ELINUX_LOG(DEBUG) << "Create the platform view: view_type = " << view_type
                    << ", id = " << view_id << ", width = " << view_width
                    << ", height = " << view_height;

  if (platform_view_factories_.find(view_type) ==
      platform_view_factories_.end()) {
    result->Error("Couldn't find the view type");
    return;
  }

  auto params = LookupEncodableMap<std::vector<uint8_t>>(arguments, kParamsKey);
  auto view = platform_view_factories_[view_type]->Create(view_id, view_width,
                                                          view_height, params);
  if (view) {
    platform_views_[view_id] = view;
    if (platform_views_.find(current_view_id_) != platform_views_.end()) {
      platform_views_[current_view_id_]->SetFocus(false);
    }
    current_view_id_ = view_id;

    result->Success(flutter::EncodableValue(view->GetTextureId()));
  } else {
    result->Error("Failed to create a platform view");
  }
}

void PlatformViewsPlugin::PlatformViewsDispose(
    const flutter::EncodableValue& arguments,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  auto view_id = LookupEncodableMap<int>(arguments, kIdKey);
  if (!view_id) {
    result->Error("Couldn't find the view id in the arguments");
    return;
  }

  ELINUX_LOG(DEBUG) << "Dispose the platform view: id = " << view_id;
  if (platform_views_.find(view_id) == platform_views_.end()) {
    result->Error("Couldn't find the view id in the arguments");
    return;
  }
  platform_views_[view_id]->Dispose();
  result->Success();
}

}  // namespace flutter
