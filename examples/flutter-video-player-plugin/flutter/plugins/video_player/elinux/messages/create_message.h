// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_CREATE_MESSAGE_H_
#define PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_CREATE_MESSAGE_H_

#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>

class CreateMessage {
 public:
  CreateMessage() = default;
  ~CreateMessage() = default;

  // Prevent copying.
  CreateMessage(CreateMessage const&) = default;
  CreateMessage& operator=(CreateMessage const&) = default;

  void SetAsset(const std::string& asset) { asset_ = asset; }

  std::string GetAsset() const { return asset_; }

  void SetUri(const std::string& uri) { uri_ = uri; }

  std::string GetUri() const { return uri_; }

  void SetPackageName(const std::string& packageName) {
    package_name_ = packageName;
  }

  std::string GetPackageName() const { return package_name_; }

  void SetFormatHint(const std::string& formatHint) {
    format_hint_ = formatHint;
  }

  std::string GetFormatHint() const { return format_hint_; }

#if 0
    public HashMap getHttpHeaders() {
      return httpHeaders;
    }

    public void setHttpHeaders(HashMap setterArg) {
      this.httpHeaders = setterArg;
    }
#endif

  flutter::EncodableValue ToMap() {
    flutter::EncodableMap map = {
        {flutter::EncodableValue("asset"), flutter::EncodableValue(asset_)},
        {flutter::EncodableValue("uri"), flutter::EncodableValue(uri_)},
        {flutter::EncodableValue("packageName"),
         flutter::EncodableValue(package_name_)},
        {flutter::EncodableValue("formatHint"),
         flutter::EncodableValue(format_hint_)}};
    return flutter::EncodableValue(map);
  }

  static CreateMessage FromMap(const flutter::EncodableValue& value) {
    CreateMessage message;
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      auto map = std::get<flutter::EncodableMap>(value);

      flutter::EncodableValue& asset = map[flutter::EncodableValue("asset")];
      if (std::holds_alternative<std::string>(asset)) {
        message.SetAsset(std::get<std::string>(asset));
      }

      flutter::EncodableValue& uri = map[flutter::EncodableValue("uri")];
      if (std::holds_alternative<std::string>(uri)) {
        message.SetUri(std::get<std::string>(uri));
      }

      flutter::EncodableValue& packageName =
          map[flutter::EncodableValue("packageName")];
      if (std::holds_alternative<std::string>(packageName)) {
        message.SetPackageName(std::get<std::string>(uri));
      }

      flutter::EncodableValue& formatHint =
          map[flutter::EncodableValue("formatHint")];
      if (std::holds_alternative<std::string>(formatHint)) {
        message.SetFormatHint(std::get<std::string>(formatHint));
      }
    }

    return message;
  }

 private:
  std::string asset_;
  std::string uri_;
  std::string package_name_;
  std::string format_hint_;
};

#endif  // PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_CREATE_MESSAGE_H_
