// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_x11.h"

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <cstring>

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
static constexpr char kWmDeleteWindow[] = "WM_DELETE_WINDOW";
static constexpr char kWindowTitle[] = "Flutter for Embedded Linux";
}  // namespace

NativeWindowX11::NativeWindowX11(Display* display,
                                 VisualID visual_id,
                                 const size_t width,
                                 const size_t height) {
  XVisualInfo visualTemplate;
  visualTemplate.visualid = visual_id;

  int visualsCount;
  auto* visual =
      XGetVisualInfo(display, VisualIDMask, &visualTemplate, &visualsCount);
  if (!visual) {
    ELINUX_LOG(ERROR) << "Failed to get Visual info.";
    return;
  }

  XSetWindowAttributes windowAttribs;
  windowAttribs.colormap = XCreateColormap(
      display, RootWindow(display, visual->screen), visual->visual, AllocNone);
  windowAttribs.border_pixel = 0;
  windowAttribs.event_mask =
      ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
      ButtonReleaseMask | PointerMotionMask | EnterWindowMask |
      LeaveWindowMask | FocusChangeMask | StructureNotifyMask;

  window_ =
      XCreateWindow(display, RootWindow(display, visual->screen), 0, 0, width,
                    height, 0, visual->depth, InputOutput, visual->visual,
                    CWBorderPixel | CWColormap | CWEventMask, &windowAttribs);
  if (!window_) {
    ELINUX_LOG(ERROR) << "Failed to the create window.";
    return;
  }

  // Receive only WM_DELETE_WINDOW message in the ClientMessage.
  auto wm_delete_window = XInternAtom(display, kWmDeleteWindow, false);
  XSetWMProtocols(display, window_, &wm_delete_window, 1);

  // Set the window title.
  {
    XTextProperty property;
    property.value =
        reinterpret_cast<unsigned char*>(const_cast<char*>(kWindowTitle));
    property.encoding = XA_STRING;
    property.format = 8;
    property.nitems = std::strlen(kWindowTitle);
    XSetWMName(display, window_, &property);
  }

  XMapWindow(display, window_);

  width_ = width;
  height_ = height;
  valid_ = true;
}

bool NativeWindowX11::Resize(const size_t width, const size_t height) {
  width_ = width;
  height_ = height;
  return true;
}

void NativeWindowX11::Destroy(Display* display) {
  if (window_) {
    XDestroyWindow(display, window_);
  }
}

}  // namespace flutter
