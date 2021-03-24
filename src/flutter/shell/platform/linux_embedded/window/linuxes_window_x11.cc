// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/linuxes_window_x11.h"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <unistd.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/context_egl_x11.h"

namespace flutter {

namespace {
static constexpr uint8_t kButtonLeft = 1;
static constexpr uint8_t kButtonMiddle = 2;
static constexpr uint8_t kButtonRight = 3;
static constexpr uint8_t kButtonBack = 4;
static constexpr uint8_t kButtonForward = 5;
}  // namespace

LinuxesWindowX11::LinuxesWindowX11(FlutterWindowMode window_mode, int32_t width,
                                   int32_t height, bool show_cursor) {
  window_mode_ = window_mode;
  current_width_ = width;
  current_height_ = height;
  show_cursor_ = show_cursor;

  display_ = XOpenDisplay(NULL);
  if (!display_) {
    LINUXES_LOG(ERROR) << "Failed to open display.";
    return;
  }

  display_valid_ = true;
}

LinuxesWindowX11::~LinuxesWindowX11() {
  display_valid_ = false;
  if (display_) {
    XSetCloseDownMode(display_, DestroyAll);
    XCloseDisplay(display_);
  }
}

bool LinuxesWindowX11::IsValid() const {
  if (!display_valid_ || !native_window_ || !render_surface_ ||
      !native_window_->IsValid() || !render_surface_->IsValid()) {
    return false;
  }
  return true;
}

bool LinuxesWindowX11::DispatchEvent() {
  auto connection = native_window_->XcbConnection();

  // TODO: support events such as keyboard input.
  xcb_generic_event_t* event;
  while ((event = xcb_poll_for_event(connection)) != NULL) {
    switch (event->response_type & ~0x80) {
      case XCB_BUTTON_PRESS: {
        auto* ev = reinterpret_cast<xcb_button_press_event_t*>(event);
        constexpr bool button_pressed = true;
        HandlePointerButtonEvent(ev->detail, button_pressed, ev->event_x,
                                 ev->event_y);
        break;
      }
      case XCB_BUTTON_RELEASE: {
        auto* ev = reinterpret_cast<xcb_button_release_event_t*>(event);
        constexpr bool button_pressed = false;
        HandlePointerButtonEvent(ev->detail, button_pressed, ev->event_x,
                                 ev->event_y);
        break;
      }
      case XCB_ENTER_NOTIFY: {
        auto* ev = reinterpret_cast<xcb_enter_notify_event_t*>(event);
        binding_handler_delegate_->OnPointerMove(ev->event_x, ev->event_y);
        break;
      }
      case XCB_MOTION_NOTIFY: {
        auto* ev = reinterpret_cast<xcb_motion_notify_event_t*>(event);
        binding_handler_delegate_->OnPointerMove(ev->event_x, ev->event_y);
        break;
      }
      case XCB_LEAVE_NOTIFY:
        binding_handler_delegate_->OnPointerLeave();
        break;
      case XCB_CONFIGURE_NOTIFY:
        break;
      case XCB_RESIZE_REQUEST: {
        auto resize = reinterpret_cast<xcb_resize_request_event_t*>(event);
        if (resize->width != current_width_ ||
            resize->height != current_height_) {
          current_width_ = resize->width;
          current_height_ = resize->height;
          if (binding_handler_delegate_) {
            binding_handler_delegate_->OnWindowSizeChanged(current_width_,
                                                           current_height_);
          }
        }
        break;
      }
      case XCB_CLIENT_MESSAGE: {
        auto message = (*reinterpret_cast<xcb_client_message_event_t*>(event))
                           .data.data32[0];
        if (message == (*native_window_->WmDeleteMessage()).atom) {
          native_window_->Destroy();
        }
        break;
      }
      case XCB_DESTROY_NOTIFY:
        // Quit the main loop.
        return false;
      default:
        break;
    }
    free(event);
    xcb_flush(connection);
  }

  return true;
}

bool LinuxesWindowX11::CreateRenderSurface(int32_t width, int32_t height) {
  native_window_ = std::make_unique<NativeWindowX11>(display_, width, height);
  if (!native_window_->IsValid()) {
    LINUXES_LOG(ERROR) << "Failed to create the native window";
    return false;
  }

  render_surface_ =
      std::make_unique<SurfaceGlX11>(std::make_unique<ContextEglX11>(
          std::make_unique<EnvironmentEgl<Display>>(display_)));
  render_surface_->SetNativeWindow(native_window_.get());

  return true;
}

void LinuxesWindowX11::DestroyRenderSurface() {
  // destroy the main surface before destroying the client window on X11.
  render_surface_ = nullptr;
  native_window_ = nullptr;
}

void LinuxesWindowX11::SetView(WindowBindingHandlerDelegate* window) {
  binding_handler_delegate_ = window;
}

LinuxesRenderSurfaceTarget* LinuxesWindowX11::GetRenderSurfaceTarget() const {
  return render_surface_.get();
}

double LinuxesWindowX11::GetDpiScale() { return current_scale_; }

PhysicalWindowBounds LinuxesWindowX11::GetPhysicalWindowBounds() {
  return {GetCurrentWidth(), GetCurrentHeight()};
}

void LinuxesWindowX11::UpdateFlutterCursor(const std::string& cursor_name) {
  // TODO: implement here
}

void LinuxesWindowX11::UpdateVirtualKeyboardStatus(const bool show) {
  // currently not supported.
}

std::string LinuxesWindowX11::GetClipboardData() { return clipboard_data_; }

void LinuxesWindowX11::SetClipboardData(const std::string& data) {
  clipboard_data_ = data;
}

void LinuxesWindowX11::HandlePointerButtonEvent(xcb_button_t button,
                                                bool button_pressed, int16_t x,
                                                int16_t y) {
  if (binding_handler_delegate_) {
    FlutterPointerMouseButtons flutter_button;
    switch (button) {
      case kButtonLeft:
        flutter_button = kFlutterPointerButtonMousePrimary;
        break;
      case kButtonRight:
        flutter_button = kFlutterPointerButtonMouseSecondary;
        break;
      case kButtonMiddle:
        flutter_button = kFlutterPointerButtonMouseMiddle;
        break;
      case kButtonBack:
        flutter_button = kFlutterPointerButtonMouseBack;
        break;
      case kButtonForward:
        flutter_button = kFlutterPointerButtonMouseForward;
        break;
      default:
        LINUXES_LOG(ERROR) << "Not expected button input: " << button;
        return;
    }
    if (button_pressed) {
      binding_handler_delegate_->OnPointerDown(x, y, flutter_button);
    } else {
      binding_handler_delegate_->OnPointerUp(x, y, flutter_button);
    }
  }
}

}  // namespace flutter
