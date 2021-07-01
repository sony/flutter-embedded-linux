// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_POSITION_MESSAGE_H_
#define PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_POSITION_MESSAGE_H_

#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>

class PositionMessage {
 public:
  PositionMessage() = default;
  ~PositionMessage() = default;

  // Prevent copying.
  PositionMessage(PositionMessage const&) = default;
  PositionMessage& operator=(PositionMessage const&) = default;

  void SetTextureId(int64_t texture_id) { texture_id_ = texture_id; }

  int64_t GetTextureId() const { return texture_id_; }

  void SetPosition(int64_t position) { position_ = position; }

  int64_t GetPosition() const { return position_; }

  flutter::EncodableValue ToMap() {
    flutter::EncodableMap toMapResult = {{flutter::EncodableValue("textureId"),
                                          flutter::EncodableValue(texture_id_)},
                                         {flutter::EncodableValue("position"),
                                          flutter::EncodableValue(position_)}};

    return flutter::EncodableValue(toMapResult);
  }

  static PositionMessage FromMap(const flutter::EncodableValue& value) {
    PositionMessage message;
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      auto map = std::get<flutter::EncodableMap>(value);

      flutter::EncodableValue& texture_id =
          map[flutter::EncodableValue("textureId")];
      if (std::holds_alternative<int32_t>(texture_id) ||
          std::holds_alternative<int64_t>(texture_id)) {
        message.SetTextureId(texture_id.LongValue());
      }

      flutter::EncodableValue& position =
          map[flutter::EncodableValue("position")];
      if (std::holds_alternative<int32_t>(position) ||
          std::holds_alternative<int64_t>(position)) {
        message.SetPosition(position.LongValue());
      }
    }

    return message;
  }

 private:
  int64_t texture_id_ = 0;
  int64_t position_ = 0;
};

#endif  // PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_POSITION_MESSAGE_H_
