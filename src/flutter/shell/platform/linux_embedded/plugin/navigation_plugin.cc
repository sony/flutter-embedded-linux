// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugin/navigation_plugin.h"

#include "flutter/shell/platform/common/json_method_codec.h"
#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
constexpr char kChannelName[] = "flutter/navigation";

constexpr char kSetInitialRouteMethod[] = "setInitialRoute";
constexpr char kPushRouteMethod[] = "pushRoute";
constexpr char kPopRouteMethod[] = "popRoute";
}  // namespace

NavigationPlugin::NavigationPlugin(BinaryMessenger* messenger)
    : channel_(std::make_unique<flutter::MethodChannel<rapidjson::Document>>(
          messenger, kChannelName, &flutter::JsonMethodCodec::GetInstance())) {}

void NavigationPlugin::SetInitialRoute(std::string route) const {
  LINUXES_LOG(DEBUG) << "SetInitialRoute = " << route;

  auto args = std::make_unique<rapidjson::Document>(rapidjson::kObjectType);
  auto& allocator = args->GetAllocator();
  args->Parse("\"" + route + "\"");
  if (args->HasParseError()) {
    LINUXES_LOG(ERROR) << "Failed to parse the initial route: " << route;
    return ;
  }
  channel_->InvokeMethod(kSetInitialRouteMethod, std::move(args));
}

void NavigationPlugin::PushRoute(std::string route) const {
  LINUXES_LOG(DEBUG) << "PushRoute = " << route;

  auto args = std::make_unique<rapidjson::Document>(rapidjson::kObjectType);
  auto& allocator = args->GetAllocator();
  args->Parse("\"" + route + "\"");
  if (args->HasParseError()) {
    LINUXES_LOG(ERROR) << "Failed to parse the route: " << route;
    return ;
  }
  channel_->InvokeMethod(kSetInitialRouteMethod, std::move(args));
}

void NavigationPlugin::PopRoute() const {
  LINUXES_LOG(DEBUG) << "PopRoute";
  channel_->InvokeMethod(kPopRouteMethod, nullptr);
}

}  // namespace flutter
