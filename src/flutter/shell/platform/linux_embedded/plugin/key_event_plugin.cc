// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugin//key_event_plugin.h"

#include <sys/mman.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>

#include "flutter/shell/platform/common/json_message_codec.h"
#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/plugin/key_event_plugin_glfw_util.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler_delegate.h"

static constexpr char kChannelName[] = "flutter/keyevent";
static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kKeyMapKey[] = "keymap";
static constexpr char kScanCodeKey[] = "scanCode";
static constexpr char kModifiersKey[] = "modifiers";
static constexpr char kTypeKey[] = "type";
static constexpr char kToolkitKey[] = "toolkit";
static constexpr char kUnicodeScalarValues[] = "unicodeScalarValues";
static constexpr char kLinuxKeyMap[] = "linux";
static constexpr char kGLFWKey[] = "glfw";
static constexpr char kKeyUp[] = "keyup";
static constexpr char kKeyDown[] = "keydown";

static constexpr char kKeyboardConfigFile[] = "/etc/default/keyboard";
static constexpr char kXkbmodelKey[] = "XKBMODEL";
static constexpr char kXkblayoutKey[] = "XKBLAYOUT";
static constexpr char kXkbvariantKey[] = "XKBVARIANT";
static constexpr char kXkboptionsKey[] = "XKBOPTIONS";

namespace flutter {

KeyeventPlugin::KeyeventPlugin(BinaryMessenger* messenger)
    : channel_(std::make_unique<BasicMessageChannel<rapidjson::Document>>(
          messenger, kChannelName, &flutter::JsonMessageCodec::GetInstance())),
      xkb_context_(xkb_context_new(XKB_CONTEXT_NO_FLAGS)) {
#if defined(DISPLAY_BACKEND_TYPE_DRM) || defined(DISPLAY_BACKEND_TYPE_EGLSTREAM)
  xkb_keymap_ = CreateKeymap(xkb_context_);
  xkb_state_ = xkb_state_new(xkb_keymap_);
#else
  xkb_keymap_ = nullptr;
  xkb_state_ = nullptr;
#endif
}

KeyeventPlugin::~KeyeventPlugin() {
  xkb_context_unref(xkb_context_);
  xkb_keymap_unref(xkb_keymap_);
  xkb_state_unref(xkb_state_);
}

void KeyeventPlugin::OnKeymap(uint32_t format, uint32_t fd, uint32_t size) {
  auto map_shm =
      reinterpret_cast<char*>(mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0));
  assert(map_shm != MAP_FAILED);
  auto xkb_keymap = xkb_keymap_new_from_string(xkb_context_, map_shm,
                                               XKB_KEYMAP_FORMAT_TEXT_V1,
                                               XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_shm, size);
  close(fd);

  auto xkb_state = xkb_state_new(xkb_keymap);
  xkb_keymap_unref(xkb_keymap_);
  xkb_state_unref(xkb_state_);
  xkb_keymap_ = xkb_keymap;
  xkb_state_ = xkb_state;
}

uint32_t KeyeventPlugin::GetCodePoint(uint32_t keycode) {
  auto sym = xkb_state_key_get_one_sym(xkb_state_, keycode + 8);
  return xkb_keysym_to_utf32(sym);
}

bool KeyeventPlugin::IsTextInputSuppressed(uint32_t code_point) {
  if (code_point) {
    auto mods = GetGlfwModifiers(xkb_mods_mask_);
    return (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT));
  }
  return false;
}

void KeyeventPlugin::OnKey(uint32_t keycode, uint32_t state) {
#if defined(DISPLAY_BACKEND_TYPE_DRM) || defined(DISPLAY_BACKEND_TYPE_EGLSTREAM)
  // We cannot get notifications of modifier keys when we use the DRM backend.
  // In this case, we need to handle it by using xkb_state_update_key.
  OnModifiers(keycode, state);
#endif
  auto unicode = GetCodePoint(keycode);
  auto mods = GetGlfwModifiers(xkb_mods_mask_);
  auto keyscancode = GetGlfwKeyScancode(keycode);
  SendKeyEvent(keyscancode, unicode, mods, state);
}

void KeyeventPlugin::OnModifiers(uint32_t mods_depressed, uint32_t mods_latched,
                                 uint32_t mods_locked, uint32_t group) {
  xkb_state_update_mask(xkb_state_, mods_depressed, mods_latched, mods_locked,
                        0, 0, group);
  xkb_mods_mask_ =
      xkb_state_serialize_mods(xkb_state_, XKB_STATE_MODS_EFFECTIVE);
}

void KeyeventPlugin::SendKeyEvent(uint32_t keycode, uint32_t unicode,
                                  uint32_t modifiers, uint32_t key_state) {
  rapidjson::Document event(rapidjson::kObjectType);
  auto& allocator = event.GetAllocator();
  event.AddMember(kKeyCodeKey, keycode, allocator);
  event.AddMember(kKeyMapKey, kLinuxKeyMap, allocator);
  event.AddMember(kToolkitKey, kGLFWKey, allocator);
  event.AddMember(kScanCodeKey, keycode, allocator);
  event.AddMember(kModifiersKey, modifiers, allocator);
  if (unicode != 0) {
    event.AddMember(kUnicodeScalarValues, unicode, allocator);
  }
  switch (key_state) {
    case FLUTTER_LINUXES_BUTTON_DOWN:
      event.AddMember(kTypeKey, kKeyDown, allocator);
      break;
    case FLUTTER_LINUXES_BUTTON_UP:
      event.AddMember(kTypeKey, kKeyUp, allocator);
      break;
    default:
      LINUXES_LOG(ERROR) << "Unknown key event action: " << key_state;
      return;
  }
  channel_->Send(event);
}

void KeyeventPlugin::OnModifiers(uint32_t keycode, uint32_t state) {
  xkb_state_update_key(xkb_state_, keycode + 8,
                       static_cast<xkb_key_direction>(state));
  xkb_mods_mask_ =
      xkb_state_serialize_mods(xkb_state_, XKB_STATE_MODS_EFFECTIVE);
}

xkb_keymap* KeyeventPlugin::CreateKeymap(xkb_context* context) {
  std::string xkbmodel = "";
  std::string xkblayout = "";
  std::string xkbvariant = "";
  std::string xkboptions = "";

  auto map = GetKeyboardConfig(kKeyboardConfigFile);
  if (map.find(kXkbmodelKey) != map.end()) {
    xkbmodel = map[kXkbmodelKey];
  }
  if (map.find(kXkblayoutKey) != map.end()) {
    xkblayout = map[kXkblayoutKey];
  }
  if (map.find(kXkbvariantKey) != map.end()) {
    xkbvariant = map[kXkbvariantKey];
  }
  if (map.find(kXkboptionsKey) != map.end()) {
    xkboptions = map[kXkboptionsKey];
  }
  struct xkb_rule_names names = {.rules = NULL,
                                 .model = xkbmodel.c_str(),
                                 .layout = xkblayout.c_str(),
                                 .variant = xkbvariant.c_str(),
                                 .options = xkboptions.c_str()};
  return xkb_keymap_new_from_names(context, &names,
                                   XKB_KEYMAP_COMPILE_NO_FLAGS);
}

std::unordered_map<std::string, std::string> KeyeventPlugin::GetKeyboardConfig(
    std::string filename) {
  std::unordered_map<std::string, std::string> map;
  char* pattern;
  auto ret =
      asprintf(&pattern, "^ *(%s|%s|%s|%s) *= *\"?([^\"]*)\"?", kXkbmodelKey,
               kXkblayoutKey, kXkbvariantKey, kXkboptionsKey);
  if (ret < 0) {
    return map;
  }

  std::regex regex(pattern);
  std::ifstream ifs(filename);
  std::string line;
  while (getline(ifs, line)) {
    std::smatch match;
    if (std::regex_search(line, match, regex)) {
      map.insert(std::make_pair(match[1], match[2]));
    }
  }

  free(pattern);
  return map;
}

}  // namespace flutter
