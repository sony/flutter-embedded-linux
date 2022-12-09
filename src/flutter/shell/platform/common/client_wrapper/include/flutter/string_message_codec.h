// Copyright 2022 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __CODE_FLUTTER_EMBEDDED_LINUX_SRC_FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STRING_MESSAGE_CODEC_H_
#define __CODE_FLUTTER_EMBEDDED_LINUX_SRC_FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STRING_MESSAGE_CODEC_H_

#include "message_codec.h"

namespace flutter {

// A message encoding/decoding mechanism for communications to/from the Flutter
// engine via UTF-8 encoded string messages.
//
// This codec is guaranteed to be compatible with the corresponding
// [StringCodec](https://api.flutter.dev/flutter/services/StringCodec-class.html)
// on the Dart side. These parts of the Flutter SDK are evolved synchronously.
class StringMessageCodec : public MessageCodec<std::string> {
 public:
  // Returns the shared instance of the codec.
  static const StringMessageCodec& GetInstance();

  ~StringMessageCodec() = default;

  // Prevent copying.
  StringMessageCodec(StringMessageCodec const&) = delete;
  StringMessageCodec& operator=(StringMessageCodec const&) = delete;

 protected:
  // Instances should be obtained via GetInstance.
  StringMessageCodec() = default;

  // |flutter::MessageCodec|
  std::unique_ptr<std::string> DecodeMessageInternal(
      const uint8_t* binary_message,
      const size_t message_size) const override;

  // |flutter::MessageCodec|
  std::unique_ptr<std::vector<uint8_t>> EncodeMessageInternal(
      const std::string& message) const override;
};

}  // namespace flutter

#endif  // __CODE_FLUTTER_EMBEDDED_LINUX_SRC_FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STRING_MESSAGE_CODEC_H_
