// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_LOOPING_MESSAGE_H_
#define PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_LOOPING_MESSAGE_H_

#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>

class LoopingMessage {
 public:
  LoopingMessage() = default;
  ~LoopingMessage() = default;

  // Prevent copying.
  LoopingMessage(LoopingMessage const&) = default;
  LoopingMessage& operator=(LoopingMessage const&) = default;

  void SetTextureId(int64_t texture_id) { texture_id_ = texture_id; }

  int64_t GetTextureId() const { return texture_id_; }

  void SetIsLooping(bool is_looping) { is_looping_ = is_looping; }

  bool GetIsLooping() const { return is_looping_; }

  flutter::EncodableValue ToMap() {
    flutter::EncodableMap map = {{flutter::EncodableValue("textureId"),
                                  flutter::EncodableValue(texture_id_)},
                                 {flutter::EncodableValue("isLooping"),
                                  flutter::EncodableValue(is_looping_)}};
    return flutter::EncodableValue(map);
  }

  static LoopingMessage FromMap(const flutter::EncodableValue& value) {
    LoopingMessage message;
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      auto map = std::get<flutter::EncodableMap>(value);

      flutter::EncodableValue& texture_id =
          map[flutter::EncodableValue("textureId")];
      if (std::holds_alternative<int32_t>(texture_id) ||
          std::holds_alternative<int64_t>(texture_id)) {
        message.SetTextureId(texture_id.LongValue());
      }

      flutter::EncodableValue& is_looping =
          map[flutter::EncodableValue("isLooping")];
      if (std::holds_alternative<bool>(is_looping)) {
        message.SetIsLooping(std::get<bool>(is_looping));
      }
    }

    return message;
  }

 private:
  int64_t texture_id_ = 0;
  bool is_looping_ = false;
};

#endif  // PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_LOOPING_MESSAGE_H_
