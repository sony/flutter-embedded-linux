// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugins/keyboard_glfw_util.h"

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>

#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
// See: https://github.com/glfw/glfw/tree/master/include/GLFW/glfw3.h
constexpr uint32_t kGlfwModifierShift = 0x0001;
constexpr uint32_t kGlfwModifierControl = 0x0002;
constexpr uint32_t kGlfwModifierAlt = 0x0004;
constexpr uint32_t kGlfwModifierSuper = 0x0008;
constexpr uint32_t kGlfwModifierCapsLock = 0x0010;
constexpr uint32_t kGlfwModifierNumLock = 0x0020;

// Printable keys
constexpr uint32_t kGlfwKeySpace = 32;
constexpr uint32_t kGlfwKeyApostrophe = 39; /* ' */
constexpr uint32_t kGlfwKeyComma = 44;      /* , */
constexpr uint32_t kGlfwKeyMinus = 45;      /* - */
constexpr uint32_t kGlfwKeyPeriod = 46;     /* . */
constexpr uint32_t kGlfwKeySlash = 47;      /* / */
constexpr uint32_t kGlfwKey0 = 48;
constexpr uint32_t kGlfwKey1 = 49;
constexpr uint32_t kGlfwKey2 = 50;
constexpr uint32_t kGlfwKey3 = 51;
constexpr uint32_t kGlfwKey4 = 52;
constexpr uint32_t kGlfwKey5 = 53;
constexpr uint32_t kGlfwKey6 = 54;
constexpr uint32_t kGlfwKey7 = 55;
constexpr uint32_t kGlfwKey8 = 56;
constexpr uint32_t kGlfwKey9 = 57;
constexpr uint32_t kGlfwKeySemicolon = 59; /* ; */
constexpr uint32_t kGlfwKeyEqual = 61;     /* = */
constexpr uint32_t kGlfwKeyA = 65;
constexpr uint32_t kGlfwKeyB = 66;
constexpr uint32_t kGlfwKeyC = 67;
constexpr uint32_t kGlfwKeyD = 68;
constexpr uint32_t kGlfwKeyE = 69;
constexpr uint32_t kGlfwKeyF = 70;
constexpr uint32_t kGlfwKeyG = 71;
constexpr uint32_t kGlfwKeyH = 72;
constexpr uint32_t kGlfwKeyI = 73;
constexpr uint32_t kGlfwKeyJ = 74;
constexpr uint32_t kGlfwKeyK = 75;
constexpr uint32_t kGlfwKeyL = 76;
constexpr uint32_t kGlfwKeyM = 77;
constexpr uint32_t kGlfwKeyN = 78;
constexpr uint32_t kGlfwKeyO = 79;
constexpr uint32_t kGlfwKeyP = 80;
constexpr uint32_t kGlfwKeyQ = 81;
constexpr uint32_t kGlfwKeyR = 82;
constexpr uint32_t kGlfwKeyS = 83;
constexpr uint32_t kGlfwKeyT = 84;
constexpr uint32_t kGlfwKeyU = 85;
constexpr uint32_t kGlfwKeyV = 86;
constexpr uint32_t kGlfwKeyW = 87;
constexpr uint32_t kGlfwKeyX = 88;
constexpr uint32_t kGlfwKeyY = 89;
constexpr uint32_t kGlfwKeyZ = 90;
constexpr uint32_t kGlfwKeyLeftBracket = 91;  /* [ */
constexpr uint32_t kGlfwKeyBackslash = 92;    /* \ */
constexpr uint32_t kGlfwKeyRightBracket = 93; /* ] */
constexpr uint32_t kGlfwKeyGraveAccent = 96;  /* ` */
// constexpr uint32_t kGlfwKeyWorld1 = 161;      /* non-US #1 */
// constexpr uint32_t kGlfwKeyWorld2 = 162;      /* non-US #2 */

// Function keys
constexpr uint32_t kGlfwKeyEscape = 256;
constexpr uint32_t kGlfwKeyEnter = 257;
constexpr uint32_t kGlfwKeyTab = 258;
constexpr uint32_t kGlfwKeyBackspace = 259;
constexpr uint32_t kGlfwKeyInsert = 260;
constexpr uint32_t kGlfwKeyDelete = 261;
constexpr uint32_t kGlfwKeyRight = 262;
constexpr uint32_t kGlfwKeyLeft = 263;
constexpr uint32_t kGlfwKeyDown = 264;
constexpr uint32_t kGlfwKeyUp = 265;
constexpr uint32_t kGlfwKeyPageUp = 266;
constexpr uint32_t kGlfwKeypageDown = 267;
constexpr uint32_t kGlfwKeyHome = 268;
constexpr uint32_t kGlfwKeyEnd = 269;
constexpr uint32_t kGlfwKeyCapsLock = 280;
constexpr uint32_t kGlfwKeyScrollLock = 281;
constexpr uint32_t kGlfwKeyNumLock = 282;
constexpr uint32_t kGlfwKeyPrintScreen = 283;
constexpr uint32_t kGlfwKeyPause = 284;
constexpr uint32_t kGlfwKeyF1 = 290;
constexpr uint32_t kGlfwKeyF2 = 291;
constexpr uint32_t kGlfwKeyF3 = 292;
constexpr uint32_t kGlfwKeyF4 = 293;
constexpr uint32_t kGlfwKeyF5 = 294;
constexpr uint32_t kGlfwKeyF6 = 295;
constexpr uint32_t kGlfwKeyF7 = 296;
constexpr uint32_t kGlfwKeyF8 = 297;
constexpr uint32_t kGlfwKeyF9 = 298;
constexpr uint32_t kGlfwKeyF10 = 299;
constexpr uint32_t kGlfwKeyF11 = 300;
constexpr uint32_t kGlfwKeyF12 = 301;
constexpr uint32_t kGlfwKeyF13 = 302;
constexpr uint32_t kGlfwKeyF14 = 303;
constexpr uint32_t kGlfwKeyF15 = 304;
constexpr uint32_t kGlfwKeyF16 = 305;
constexpr uint32_t kGlfwKeyF17 = 306;
constexpr uint32_t kGlfwKeyF18 = 307;
constexpr uint32_t kGlfwKeyF19 = 308;
constexpr uint32_t kGlfwKeyF20 = 309;
constexpr uint32_t kGlfwKeyF21 = 310;
constexpr uint32_t kGlfwKeyF22 = 311;
constexpr uint32_t kGlfwKeyF23 = 312;
constexpr uint32_t kGlfwKeyF24 = 313;
constexpr uint32_t kGlfwKeyF25 = 314;
constexpr uint32_t kGlfwKeyKp0 = 320;
constexpr uint32_t kGlfwKeyKp1 = 321;
constexpr uint32_t kGlfwKeyKp2 = 322;
constexpr uint32_t kGlfwKeyKp3 = 323;
constexpr uint32_t kGlfwKeyKp4 = 324;
constexpr uint32_t kGlfwKeyKp5 = 325;
constexpr uint32_t kGlfwKeyKp6 = 326;
constexpr uint32_t kGlfwKeyKp7 = 327;
constexpr uint32_t kGlfwKeyKp8 = 328;
constexpr uint32_t kGlfwKeyKp9 = 329;
constexpr uint32_t kGlfwKeyKpDecimal = 330;
constexpr uint32_t kGlfwKeyKpDivide = 331;
constexpr uint32_t kGlfwKeyKpMultiply = 332;
constexpr uint32_t kGlfwKeyKpSubtract = 333;
constexpr uint32_t kGlfwKeyKpAdd = 334;
constexpr uint32_t kGlfwKeyKpEnter = 335;
constexpr uint32_t kGlfwKeyKpEqual = 336;
constexpr uint32_t kGlfwKeyLeftShift = 340;
constexpr uint32_t kGlfwKeyLeftControl = 341;
constexpr uint32_t kGlfwKeyLeftAlt = 342;
constexpr uint32_t kGlfwKeyLeftSuper = 343;
constexpr uint32_t kGlfwKeyRightShift = 344;
constexpr uint32_t kGlfwKeyRightControl = 345;
constexpr uint32_t kGlfwKeyRightAlt = 346;
constexpr uint32_t kGlfwKeyRightSuper = 347;
constexpr uint32_t kGlfwKeyMenu = 348;
}  // namespace

uint32_t GetGlfwModifiers(xkb_keymap* xkb_keymap,
                          xkb_mod_mask_t& xkb_mod_mask) {
  const xkb_mod_mask_t xkb_shift_key =
      1 << xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_SHIFT);
  const xkb_mod_mask_t xkb_ctrl_key =
      1 << xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_CTRL);
  const xkb_mod_mask_t xkb_alt_key =
      1 << xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_ALT);
  const xkb_mod_mask_t xkb_super_key =
      1 << xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_LOGO);
  const xkb_mod_mask_t xkb_caps_lock_key =
      1 << xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_CAPS);
  const xkb_mod_mask_t xkb_num_lock_key =
      1 << xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_NUM);

  uint32_t mods = 0;
  if (xkb_mod_mask & xkb_shift_key) {
    mods |= kGlfwModifierShift;
  }
  if (xkb_mod_mask & xkb_ctrl_key) {
    mods |= kGlfwModifierControl;
  }
  if (xkb_mod_mask & xkb_alt_key) {
    mods |= kGlfwModifierAlt;
  }
  if (xkb_mod_mask & xkb_super_key) {
    mods |= kGlfwModifierSuper;
  }
  if (xkb_mod_mask & xkb_caps_lock_key) {
    mods |= kGlfwModifierCapsLock;
  }
  if (xkb_mod_mask & xkb_num_lock_key) {
    mods |= kGlfwModifierNumLock;
  }
  return mods;
}

uint32_t GetGlfwKeycode(uint32_t xkb_keycode) {
  static const std::unordered_map<uint32_t, uint32_t> keycode_to_glfwkey_map = {
      {KEY_GRAVE, kGlfwKeyGraveAccent},
      {KEY_1, kGlfwKey1},
      {KEY_2, kGlfwKey2},
      {KEY_3, kGlfwKey3},
      {KEY_4, kGlfwKey4},
      {KEY_5, kGlfwKey5},
      {KEY_6, kGlfwKey6},
      {KEY_7, kGlfwKey7},
      {KEY_8, kGlfwKey8},
      {KEY_9, kGlfwKey9},
      {KEY_0, kGlfwKey0},
      {KEY_SPACE, kGlfwKeySpace},
      {KEY_MINUS, kGlfwKeyMinus},
      {KEY_EQUAL, kGlfwKeyEqual},
      {KEY_Q, kGlfwKeyQ},
      {KEY_W, kGlfwKeyW},
      {KEY_E, kGlfwKeyE},
      {KEY_R, kGlfwKeyR},
      {KEY_T, kGlfwKeyT},
      {KEY_Y, kGlfwKeyY},
      {KEY_U, kGlfwKeyU},
      {KEY_I, kGlfwKeyI},
      {KEY_O, kGlfwKeyO},
      {KEY_P, kGlfwKeyP},
      {KEY_LEFTBRACE, kGlfwKeyLeftBracket},
      {KEY_RIGHTBRACE, kGlfwKeyRightBracket},
      {KEY_A, kGlfwKeyA},
      {KEY_S, kGlfwKeyS},
      {KEY_D, kGlfwKeyD},
      {KEY_F, kGlfwKeyF},
      {KEY_G, kGlfwKeyG},
      {KEY_H, kGlfwKeyH},
      {KEY_J, kGlfwKeyJ},
      {KEY_K, kGlfwKeyK},
      {KEY_L, kGlfwKeyL},
      {KEY_SEMICOLON, kGlfwKeySemicolon},
      {KEY_APOSTROPHE, kGlfwKeyApostrophe},
      {KEY_Z, kGlfwKeyZ},
      {KEY_X, kGlfwKeyX},
      {KEY_C, kGlfwKeyC},
      {KEY_V, kGlfwKeyV},
      {KEY_B, kGlfwKeyB},
      {KEY_N, kGlfwKeyN},
      {KEY_M, kGlfwKeyM},
      {KEY_COMMA, kGlfwKeyComma},
      {KEY_DOT, kGlfwKeyPeriod},
      {KEY_SLASH, kGlfwKeySlash},
      {KEY_BACKSLASH, kGlfwKeyBackslash},
      {KEY_ESC, kGlfwKeyEscape},
      {KEY_TAB, kGlfwKeyTab},
      {KEY_LEFTSHIFT, kGlfwKeyLeftShift},
      {KEY_RIGHTSHIFT, kGlfwKeyRightShift},
      {KEY_LEFTCTRL, kGlfwKeyLeftControl},
      {KEY_RIGHTCTRL, kGlfwKeyRightControl},
      {KEY_LEFTALT, kGlfwKeyLeftAlt},
      {KEY_RIGHTALT, kGlfwKeyRightAlt},
      {KEY_LEFTMETA, kGlfwKeyLeftSuper},
      {KEY_RIGHTMETA, kGlfwKeyRightSuper},
      {KEY_MENU, kGlfwKeyMenu},
      {KEY_NUMLOCK, kGlfwKeyNumLock},
      {KEY_CAPSLOCK, kGlfwKeyCapsLock},
      {KEY_PRINT, kGlfwKeyPrintScreen},
      {KEY_SCROLLLOCK, kGlfwKeyScrollLock},
      {KEY_PAUSE, kGlfwKeyPause},
      {KEY_DELETE, kGlfwKeyDelete},
      {KEY_BACKSPACE, kGlfwKeyBackspace},
      {KEY_ENTER, kGlfwKeyEnter},
      {KEY_HOME, kGlfwKeyHome},
      {KEY_END, kGlfwKeyEnd},
      {KEY_PAGEUP, kGlfwKeyPageUp},
      {KEY_PAGEDOWN, kGlfwKeypageDown},
      {KEY_INSERT, kGlfwKeyInsert},
      {KEY_LEFT, kGlfwKeyLeft},
      {KEY_RIGHT, kGlfwKeyRight},
      {KEY_DOWN, kGlfwKeyDown},
      {KEY_UP, kGlfwKeyUp},
      {KEY_F1, kGlfwKeyF1},
      {KEY_F2, kGlfwKeyF2},
      {KEY_F3, kGlfwKeyF3},
      {KEY_F4, kGlfwKeyF4},
      {KEY_F5, kGlfwKeyF5},
      {KEY_F6, kGlfwKeyF6},
      {KEY_F7, kGlfwKeyF7},
      {KEY_F8, kGlfwKeyF8},
      {KEY_F9, kGlfwKeyF9},
      {KEY_F10, kGlfwKeyF10},
      {KEY_F11, kGlfwKeyF11},
      {KEY_F12, kGlfwKeyF12},
      {KEY_F13, kGlfwKeyF13},
      {KEY_F14, kGlfwKeyF14},
      {KEY_F15, kGlfwKeyF15},
      {KEY_F16, kGlfwKeyF16},
      {KEY_F17, kGlfwKeyF17},
      {KEY_F18, kGlfwKeyF18},
      {KEY_F19, kGlfwKeyF19},
      {KEY_F20, kGlfwKeyF20},
      {KEY_F21, kGlfwKeyF21},
      {KEY_F22, kGlfwKeyF22},
      {KEY_F23, kGlfwKeyF23},
      {KEY_F24, kGlfwKeyF24},
      {KEY_KPSLASH, kGlfwKeyKpDivide},
      {KEY_KPDOT, kGlfwKeyKpMultiply},
      {KEY_KPMINUS, kGlfwKeyKpSubtract},
      {KEY_KPPLUS, kGlfwKeyKpAdd},
      {KEY_KP0, kGlfwKeyKp0},
      {KEY_KP1, kGlfwKeyKp1},
      {KEY_KP2, kGlfwKeyKp2},
      {KEY_KP3, kGlfwKeyKp3},
      {KEY_KP4, kGlfwKeyKp4},
      {KEY_KP5, kGlfwKeyKp5},
      {KEY_KP6, kGlfwKeyKp6},
      {KEY_KP7, kGlfwKeyKp7},
      {KEY_KP8, kGlfwKeyKp8},
      {KEY_KP9, kGlfwKeyKp9},
      {KEY_KPCOMMA, kGlfwKeyKpDecimal},
      {KEY_KPEQUAL, kGlfwKeyKpEqual},
      {KEY_KPENTER, kGlfwKeyKpEnter},
  };

  if (keycode_to_glfwkey_map.find(xkb_keycode) !=
      keycode_to_glfwkey_map.end()) {
    return keycode_to_glfwkey_map.at(xkb_keycode);
  }
  LINUXES_LOG(ERROR) << "Unknown keycode: " << xkb_keycode;
  return xkb_keycode;
}

}  // namespace flutter
