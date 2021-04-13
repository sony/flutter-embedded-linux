// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugin/key_event_plugin_glfw_util.h"

#include <linux/input-event-codes.h>

#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
constexpr uint32_t kXModifierShift = 0x0001;
constexpr uint32_t kXModifierCapsLock = 0x0002;
constexpr uint32_t kXModifierControl = 0x0004;
constexpr uint32_t kXModifierAlt = 0x0008;
constexpr uint32_t kXModifierNumLock = 0x0010;
constexpr uint32_t kXModifierSuper = 0x0040;
}  // namespace

uint32_t GetGlfwModifiers(xkb_mod_mask_t xkb_mod_mask) {
  uint32_t mods = 0;

  if (xkb_mod_mask & kXModifierShift) {
    mods |= GLFW_MOD_SHIFT;
  }
  if (xkb_mod_mask & kXModifierControl) {
    mods |= GLFW_MOD_CONTROL;
  }
  if (xkb_mod_mask & kXModifierAlt) {
    mods |= GLFW_MOD_ALT;
  }
  if (xkb_mod_mask & kXModifierSuper) {
    mods |= GLFW_MOD_SUPER;
  }
  if (xkb_mod_mask & kXModifierCapsLock) {
    mods |= GLFW_MOD_CAPS_LOCK;
  }
  if (xkb_mod_mask & kXModifierNumLock) {
    mods |= GLFW_MOD_NUM_LOCK;
  }
  return mods;
}

uint32_t GetGlfwKeyScancode(uint32_t keycode) {
  static const std::unordered_map<uint32_t, uint32_t> keycode_to_glfwkey_map = {
      {KEY_GRAVE, GLFW_KEY_GRAVE_ACCENT},
      {KEY_1, GLFW_KEY_1},
      {KEY_2, GLFW_KEY_2},
      {KEY_3, GLFW_KEY_3},
      {KEY_4, GLFW_KEY_4},
      {KEY_5, GLFW_KEY_5},
      {KEY_6, GLFW_KEY_6},
      {KEY_7, GLFW_KEY_7},
      {KEY_8, GLFW_KEY_8},
      {KEY_9, GLFW_KEY_9},
      {KEY_0, GLFW_KEY_0},
      {KEY_SPACE, GLFW_KEY_SPACE},
      {KEY_MINUS, GLFW_KEY_MINUS},
      {KEY_EQUAL, GLFW_KEY_EQUAL},
      {KEY_Q, GLFW_KEY_Q},
      {KEY_W, GLFW_KEY_W},
      {KEY_E, GLFW_KEY_E},
      {KEY_R, GLFW_KEY_R},
      {KEY_T, GLFW_KEY_T},
      {KEY_Y, GLFW_KEY_Y},
      {KEY_U, GLFW_KEY_U},
      {KEY_I, GLFW_KEY_I},
      {KEY_O, GLFW_KEY_O},
      {KEY_P, GLFW_KEY_P},
      {KEY_LEFTBRACE, GLFW_KEY_LEFT_BRACKET},
      {KEY_RIGHTBRACE, GLFW_KEY_RIGHT_BRACKET},
      {KEY_A, GLFW_KEY_A},
      {KEY_S, GLFW_KEY_S},
      {KEY_D, GLFW_KEY_D},
      {KEY_F, GLFW_KEY_F},
      {KEY_G, GLFW_KEY_G},
      {KEY_H, GLFW_KEY_H},
      {KEY_J, GLFW_KEY_J},
      {KEY_K, GLFW_KEY_K},
      {KEY_L, GLFW_KEY_L},
      {KEY_SEMICOLON, GLFW_KEY_SEMICOLON},
      {KEY_APOSTROPHE, GLFW_KEY_APOSTROPHE},
      {KEY_Z, GLFW_KEY_Z},
      {KEY_X, GLFW_KEY_X},
      {KEY_C, GLFW_KEY_C},
      {KEY_V, GLFW_KEY_V},
      {KEY_B, GLFW_KEY_B},
      {KEY_N, GLFW_KEY_N},
      {KEY_M, GLFW_KEY_M},
      {KEY_COMMA, GLFW_KEY_COMMA},
      {KEY_DOT, GLFW_KEY_PERIOD},
      {KEY_SLASH, GLFW_KEY_SLASH},
      {KEY_BACKSLASH, GLFW_KEY_BACKSLASH},
      {KEY_ESC, GLFW_KEY_ESCAPE},
      {KEY_TAB, GLFW_KEY_TAB},
      {KEY_LEFTSHIFT, GLFW_KEY_LEFT_SHIFT},
      {KEY_RIGHTSHIFT, GLFW_KEY_RIGHT_SHIFT},
      {KEY_LEFTCTRL, GLFW_KEY_LEFT_CONTROL},
      {KEY_RIGHTCTRL, GLFW_KEY_RIGHT_CONTROL},
      {KEY_LEFTALT, GLFW_KEY_LEFT_ALT},
      {KEY_RIGHTALT, GLFW_KEY_RIGHT_ALT},
      {KEY_LEFTMETA, GLFW_KEY_LEFT_SUPER},
      {KEY_RIGHTMETA, GLFW_KEY_RIGHT_SUPER},
      {KEY_MENU, GLFW_KEY_MENU},
      {KEY_NUMLOCK, GLFW_KEY_NUM_LOCK},
      {KEY_CAPSLOCK, GLFW_KEY_CAPS_LOCK},
      {KEY_PRINT, GLFW_KEY_PRINT_SCREEN},
      {KEY_SCROLLLOCK, GLFW_KEY_SCROLL_LOCK},
      {KEY_PAUSE, GLFW_KEY_PAUSE},
      {KEY_DELETE, GLFW_KEY_DELETE},
      {KEY_BACKSPACE, GLFW_KEY_BACKSPACE},
      {KEY_ENTER, GLFW_KEY_ENTER},
      {KEY_HOME, GLFW_KEY_HOME},
      {KEY_END, GLFW_KEY_END},
      {KEY_PAGEUP, GLFW_KEY_PAGE_UP},
      {KEY_PAGEDOWN, GLFW_KEY_PAGE_DOWN},
      {KEY_INSERT, GLFW_KEY_INSERT},
      {KEY_LEFT, GLFW_KEY_LEFT},
      {KEY_RIGHT, GLFW_KEY_RIGHT},
      {KEY_DOWN, GLFW_KEY_DOWN},
      {KEY_UP, GLFW_KEY_UP},
      {KEY_F1, GLFW_KEY_F1},
      {KEY_F2, GLFW_KEY_F2},
      {KEY_F3, GLFW_KEY_F3},
      {KEY_F4, GLFW_KEY_F4},
      {KEY_F5, GLFW_KEY_F5},
      {KEY_F6, GLFW_KEY_F6},
      {KEY_F7, GLFW_KEY_F7},
      {KEY_F8, GLFW_KEY_F8},
      {KEY_F9, GLFW_KEY_F9},
      {KEY_F10, GLFW_KEY_F10},
      {KEY_F11, GLFW_KEY_F11},
      {KEY_F12, GLFW_KEY_F12},
      {KEY_F13, GLFW_KEY_F13},
      {KEY_F14, GLFW_KEY_F14},
      {KEY_F15, GLFW_KEY_F15},
      {KEY_F16, GLFW_KEY_F16},
      {KEY_F17, GLFW_KEY_F17},
      {KEY_F18, GLFW_KEY_F18},
      {KEY_F19, GLFW_KEY_F19},
      {KEY_F20, GLFW_KEY_F20},
      {KEY_F21, GLFW_KEY_F21},
      {KEY_F22, GLFW_KEY_F22},
      {KEY_F23, GLFW_KEY_F23},
      {KEY_F24, GLFW_KEY_F24},
      {KEY_KPSLASH, GLFW_KEY_KP_DIVIDE},
      {KEY_KPDOT, GLFW_KEY_KP_MULTIPLY},
      {KEY_KPMINUS, GLFW_KEY_KP_SUBTRACT},
      {KEY_KPPLUS, GLFW_KEY_KP_ADD},
      {KEY_KP0, GLFW_KEY_KP_0},
      {KEY_KP1, GLFW_KEY_KP_1},
      {KEY_KP2, GLFW_KEY_KP_2},
      {KEY_KP3, GLFW_KEY_KP_3},
      {KEY_KP4, GLFW_KEY_KP_4},
      {KEY_KP5, GLFW_KEY_KP_5},
      {KEY_KP6, GLFW_KEY_KP_6},
      {KEY_KP7, GLFW_KEY_KP_7},
      {KEY_KP8, GLFW_KEY_KP_8},
      {KEY_KP9, GLFW_KEY_KP_9},
      {KEY_KPCOMMA, GLFW_KEY_KP_DECIMAL},
      {KEY_KPEQUAL, GLFW_KEY_KP_EQUAL},
      {KEY_KPENTER, GLFW_KEY_KP_ENTER},
  };

  if (keycode_to_glfwkey_map.find(keycode) != keycode_to_glfwkey_map.end()) {
    return keycode_to_glfwkey_map.at(keycode);
  }
  LINUXES_LOG(ERROR) << "Unknown keycode: " << keycode;
  return keycode;
}

}  // namespace flutter
