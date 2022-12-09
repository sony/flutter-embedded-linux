// Copyright 2022 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/string_message_codec.h"

#include <string>

namespace flutter {

// static
const StringMessageCodec& StringMessageCodec::GetInstance() {
  static StringMessageCodec sInstance;
  return sInstance;
}

std::unique_ptr<std::vector<uint8_t>> StringMessageCodec::EncodeMessageInternal(
    const std::string& message) const {
  return std::make_unique<std::vector<uint8_t>>(message.begin(), message.end());
}

std::unique_ptr<std::string> StringMessageCodec::DecodeMessageInternal(
    const uint8_t* binary_message,
    const size_t message_size) const {
  if (!binary_message) {
    return nullptr;
  }

  auto raw_message = reinterpret_cast<const char*>(binary_message);

  return std::make_unique<std::string>(raw_message, message_size);
}

}  // namespace flutter
