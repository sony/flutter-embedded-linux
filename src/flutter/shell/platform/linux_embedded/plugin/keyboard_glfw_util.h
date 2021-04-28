// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_KEYBOARD_GLFW_UTIL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_KEYBOARD_GLFW_UTIL_H_

#include <xkbcommon/xkbcommon.h>

namespace flutter {

// Converts modifiers for xkb to modifiers for GLFW.
uint32_t GetGlfwModifiers(const xkb_keymap* xkb_keymap,
                          const xkb_mod_mask_t& xkb_mod_mask);

// Converts xkb keycode to GLFW keycode.
uint32_t GetGlfwKeycode(uint32_t xkb_keycode);

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PLUGIN_KEYBOARD_GLFW_UTIL_H_
