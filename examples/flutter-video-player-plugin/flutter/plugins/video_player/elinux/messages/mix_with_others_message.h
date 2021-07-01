// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_MIX_WITH_OTHERS_MESSAGE_H_
#define PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_MIX_WITH_OTHERS_MESSAGE_H_

#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>

class MixWithOthersMessage {
 public:
  MixWithOthersMessage() = default;
  ~MixWithOthersMessage() = default;

  // Prevent copying.
  MixWithOthersMessage(MixWithOthersMessage const&) = default;
  MixWithOthersMessage& operator=(MixWithOthersMessage const&) = default;

  void SetMixWithOthers(bool mixWithOthers) {
    mix_with_others_ = mixWithOthers;
  }

  bool GetMixWithOthers() const { return mix_with_others_; }

  flutter::EncodableValue ToMap() {
    flutter::EncodableMap map = {{flutter::EncodableValue("mixWithOthers"),
                                  flutter::EncodableValue(mix_with_others_)}};

    return flutter::EncodableValue(map);
  }

  static MixWithOthersMessage FromMap(const flutter::EncodableValue &value) {
    MixWithOthersMessage message;
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      auto map = std::get<flutter::EncodableMap>(value);

      flutter::EncodableValue& mixWithOthers =
          map[flutter::EncodableValue("mixWithOthers")];
      if (std::holds_alternative<bool>(mixWithOthers)) {
        message.SetMixWithOthers(std::get<bool>(mixWithOthers));
      }
    }

    return message;
  }

 private:
  bool mix_with_others_ = false;
};

#endif  // PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_MESSAGES_MIX_WITH_OTHERS_MESSAGE_H_
