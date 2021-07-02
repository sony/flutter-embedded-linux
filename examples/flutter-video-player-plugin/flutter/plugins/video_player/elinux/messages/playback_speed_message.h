// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_PLAYBACK_SPEED_MESSAGE_H_
#define PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_PLAYBACK_SPEED_MESSAGE_H_

#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>

class PlaybackSpeedMessage {
 public:
  PlaybackSpeedMessage() = default;
  ~PlaybackSpeedMessage() = default;

  // Prevent copying.
  PlaybackSpeedMessage(PlaybackSpeedMessage const&) = default;
  PlaybackSpeedMessage& operator=(PlaybackSpeedMessage const&) = default;

  void SetTextureId(int64_t texture_id) { texture_id_ = texture_id; }

  int64_t GetTextureId() const { return texture_id_; }

  void SetSpeed(double speed) { speed_ = speed; }

  double GetSpeed() const { return speed_; }

  flutter::EncodableValue ToMap() {
    flutter::EncodableMap map = {
        {flutter::EncodableValue("textureId"),
         flutter::EncodableValue(texture_id_)},
        {flutter::EncodableValue("speed"), flutter::EncodableValue(speed_)}};
    return flutter::EncodableValue(map);
  }

  static PlaybackSpeedMessage FromMap(const flutter::EncodableValue& value) {
    PlaybackSpeedMessage message;
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      auto map = std::get<flutter::EncodableMap>(value);

      flutter::EncodableValue& texture_id =
          map[flutter::EncodableValue("textureId")];
      if (std::holds_alternative<int32_t>(texture_id) ||
          std::holds_alternative<int64_t>(texture_id)) {
        message.SetTextureId(texture_id.LongValue());
      }

      flutter::EncodableValue& speed = map[flutter::EncodableValue("speed")];
      if (std::holds_alternative<double>(speed)) {
        message.SetSpeed(std::get<double>(speed));
      }
    }

    return message;
  }

 private:
  int64_t texture_id_ = 0;
  double speed_ = 1.0;
};

#endif  // PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_PLAYBACK_SPEED_MESSAGE_H_
