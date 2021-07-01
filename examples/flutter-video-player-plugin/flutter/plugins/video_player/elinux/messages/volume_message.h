// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_VOLUME_MESSAGE_H_
#define PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_VOLUME_MESSAGE_H_

#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>

class VolumeMessage {
 public:
  VolumeMessage() = default;
  ~VolumeMessage() = default;

  // Prevent copying.
  VolumeMessage(VolumeMessage const&) = default;
  VolumeMessage& operator=(VolumeMessage const&) = default;

  void SetTextureId(int64_t texture_id) { texture_id_ = texture_id; }

  int64_t GetTextureId() const { return texture_id_; }

  void SetVolume(double volume) { volume_ = volume; }

  double GetVolume() const { return volume_; }

  flutter::EncodableValue ToMap() {
    flutter::EncodableMap map = {
        {flutter::EncodableValue("textureId"),
         flutter::EncodableValue(texture_id_)},
        {flutter::EncodableValue("volume"), flutter::EncodableValue(volume_)}};
    return flutter::EncodableValue(map);
  }

  static VolumeMessage FromMap(const flutter::EncodableValue& value) {
    VolumeMessage message;
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      auto map = std::get<flutter::EncodableMap>(value);

      flutter::EncodableValue& texture_id =
          map[flutter::EncodableValue("textureId")];
      if (std::holds_alternative<int32_t>(texture_id) ||
          std::holds_alternative<int64_t>(texture_id)) {
        message.SetTextureId(texture_id.LongValue());
      }

      flutter::EncodableValue& volume = map[flutter::EncodableValue("volume")];
      if (std::holds_alternative<double>(volume)) {
        message.SetVolume(std::get<double>(volume));
      }
    }

    return message;
  }

 private:
  int64_t texture_id_ = 0;
  double volume_ = 0;
};

#endif  // PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_VOLUME_MESSAGE_H_
