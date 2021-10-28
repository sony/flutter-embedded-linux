// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_KEY_EVENT_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_KEY_EVENT_PLUGIN_H_

#include <rapidjson/document.h>
#include <xkbcommon/xkbcommon.h>

#include <memory>
#include <unordered_map>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/basic_message_channel.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"

namespace flutter {

class KeyeventPlugin {
 public:
  KeyeventPlugin(BinaryMessenger* messenger);
  ~KeyeventPlugin();

  void OnKeymap(uint32_t format, uint32_t fd, uint32_t size);

  void OnKey(uint32_t keycode, bool pressed);

  void OnModifiers(uint32_t mods_depressed,
                   uint32_t mods_latched,
                   uint32_t mods_locked,
                   uint32_t group);

  uint32_t GetCodePoint(uint32_t keycode);

  bool IsTextInputSuppressed(uint32_t code_point);

 private:
  void SendKeyEvent(uint32_t keycode,
                    uint32_t unicode,
                    uint32_t modifiers,
                    bool pressed);
  void OnModifiers(uint32_t keycode, bool pressed);
  xkb_keymap* CreateKeymap(xkb_context* context);
  std::unordered_map<std::string, std::string> GetKeyboardConfig(
      std::string filename);

  std::unique_ptr<BasicMessageChannel<rapidjson::Document>> channel_;
  xkb_context* xkb_context_;
  xkb_state* xkb_state_;
  xkb_keymap* xkb_keymap_;
  xkb_mod_mask_t xkb_mods_mask_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGINS_KEY_EVENT_PLUGIN_H_
