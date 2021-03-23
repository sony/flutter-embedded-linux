// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/native_window_x11.h"

#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
  static constexpr char kXcbWmProtocols[] = "WM_PROTOCOLS";
  static constexpr char kXcbWmDeleteWindow[] = "WM_DELETE_WINDOW";
}

NativeWindowX11::NativeWindowX11(Display* display, const size_t width,
                                 const size_t height) {
  xcb_connection_ = XGetXCBConnection(display);
  if (!xcb_connection_) {
    LINUXES_LOG(ERROR) << "Failed to get xcb connection.";
    return;
  }

  auto* setup = xcb_get_setup(xcb_connection_);
  auto screen_iterator = xcb_setup_roots_iterator(setup).data;
  if (!screen_iterator) {
    LINUXES_LOG(ERROR) << "Failed to setup_roots.";
    return;
  }

  window_ = xcb_generate_id(xcb_connection_);

  uint32_t mask = XCB_CW_EVENT_MASK;
  uint32_t valwin[1] = {XCB_EVENT_MASK_EXPOSURE |
                        XCB_EVENT_MASK_RESIZE_REDIRECT |
                        XCB_EVENT_MASK_STRUCTURE_NOTIFY};
  xcb_create_window(xcb_connection_, XCB_COPY_FROM_PARENT, window_,
                    screen_iterator->root, 0, 0, width, height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, screen_iterator->root_visual,
                    mask, valwin);
  xcb_flush(xcb_connection_);

  // Enable message when press close button.
  auto protocols_cookie =
      xcb_intern_atom(xcb_connection_, 1, 12, kXcbWmProtocols);
  auto* protocol_reply =
      xcb_intern_atom_reply(xcb_connection_, protocols_cookie, 0);
  auto delete_cookie =
      xcb_intern_atom(xcb_connection_, 0, 16, kXcbWmDeleteWindow);
  reply_delete_ = xcb_intern_atom_reply(xcb_connection_, delete_cookie, 0);
  xcb_change_property(xcb_connection_, XCB_PROP_MODE_REPLACE, window_,
                      (*protocol_reply).atom, 4, 32, 1, &(*reply_delete_).atom);
  free(protocol_reply);

  xcb_map_window(xcb_connection_, window_);
  xcb_flush(xcb_connection_);

  valid_ = true;
}

NativeWindowX11::~NativeWindowX11() {
  if (reply_delete_) {
    free(reply_delete_);
    reply_delete_ = nullptr;
  }
}

bool NativeWindowX11::Resize(const size_t width, const size_t height) const {
  if (!valid_) {
    LINUXES_LOG(ERROR) << "Failed to resize the window.";
    return false;
  }
  return true;
}

void NativeWindowX11::Destroy() {
  if (xcb_connection_) {
    xcb_destroy_window(xcb_connection_, window_);
  }
}

}  // namespace flutter
