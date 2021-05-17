// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/linuxes_window_x11.h"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <unistd.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"

namespace flutter {

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
  while (XPending(display_)) {
    XEvent event;
    XNextEvent(display_, &event);
    switch (event.type) {
      case EnterNotify:
      case MotionNotify:
        if (binding_handler_delegate_) {
          binding_handler_delegate_->OnPointerMove(event.xbutton.x,
                                                   event.xbutton.y);
        }
        break;
      case LeaveNotify:
        if (binding_handler_delegate_) {
          binding_handler_delegate_->OnPointerLeave();
        }
        break;
      case ButtonPress: {
        constexpr bool button_pressed = true;
        HandlePointerButtonEvent(event.xbutton.button, button_pressed,
                                 event.xbutton.x, event.xbutton.y);
      } break;
      case ButtonRelease: {
        constexpr bool button_pressed = false;
        HandlePointerButtonEvent(event.xbutton.button, button_pressed,
                                 event.xbutton.x, event.xbutton.y);
      } break;
      case KeyPress:
        if (binding_handler_delegate_) {
          constexpr bool pressed = true;
          binding_handler_delegate_->OnKey(event.xkey.keycode - 8, pressed);
        }
        break;
      case KeyRelease:
        if (binding_handler_delegate_) {
          constexpr bool pressed = false;
          binding_handler_delegate_->OnKey(event.xkey.keycode - 8, pressed);
        }
        break;
      case ConfigureNotify: {
        if (((event.xconfigure.width != current_width_) ||
             (event.xconfigure.height != current_height_))) {
          current_width_ = event.xconfigure.width;
          current_height_ = event.xconfigure.height;
          if (binding_handler_delegate_) {
            binding_handler_delegate_->OnWindowSizeChanged(current_width_,
                                                           current_height_);
          }
        }
      } break;
      case ClientMessage:
        native_window_->Destroy(display_);
        break;
      case DestroyNotify:
        // Quit the main loop.
        return false;
      default:
        break;
    }
  }
  return true;
}

bool LinuxesWindowX11::CreateRenderSurface(int32_t width, int32_t height) {
  auto context_egl =
      std::make_unique<ContextEgl>(std::make_unique<EnvironmentEgl>(display_));

  native_window_ = std::make_unique<NativeWindowX11>(
      display_, context_egl->GetAttrib(EGL_NATIVE_VISUAL_ID), width, height);
  if (!native_window_->IsValid()) {
    LINUXES_LOG(ERROR) << "Failed to create the native window";
    return false;
  }

  render_surface_ = std::make_unique<SurfaceGl>(std::move(context_egl));
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

void LinuxesWindowX11::HandlePointerButtonEvent(uint32_t button,
                                                bool button_pressed, int16_t x,
                                                int16_t y) {
  if (binding_handler_delegate_) {
    FlutterPointerMouseButtons flutter_button;
    switch (button) {
      case Button1:
        flutter_button = kFlutterPointerButtonMousePrimary;
        break;
      case Button2:
        flutter_button = kFlutterPointerButtonMouseMiddle;
        break;
      case Button3:
        flutter_button = kFlutterPointerButtonMouseSecondary;
        break;
      case Button4:
        flutter_button = kFlutterPointerButtonMouseBack;
        break;
      case Button5:
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
