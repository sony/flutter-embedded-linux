// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/elinux_window_x11.h"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <unistd.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"

namespace flutter {

ELinuxWindowX11::ELinuxWindowX11(FlutterDesktopViewProperties view_properties) {
  view_properties_ = view_properties;

  display_ = XOpenDisplay(NULL);
  if (!display_) {
    ELINUX_LOG(ERROR) << "Failed to open display.";
    return;
  }

  display_valid_ = true;
}

ELinuxWindowX11::~ELinuxWindowX11() {
  display_valid_ = false;
  if (display_) {
    XSetCloseDownMode(display_, DestroyAll);
    XCloseDisplay(display_);
  }
}

bool ELinuxWindowX11::IsValid() const {
  if (!display_valid_ || !native_window_ || !render_surface_ ||
      !native_window_->IsValid() || !render_surface_->IsValid()) {
    return false;
  }
  return true;
}

bool ELinuxWindowX11::DispatchEvent() {
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
        if (((event.xconfigure.width != view_properties_.width) ||
             (event.xconfigure.height != view_properties_.height))) {
          view_properties_.width = event.xconfigure.width;
          view_properties_.height = event.xconfigure.height;
          if (binding_handler_delegate_) {
            binding_handler_delegate_->OnWindowSizeChanged(
                view_properties_.width, view_properties_.height);
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

bool ELinuxWindowX11::CreateRenderSurface(int32_t width, int32_t height) {
  auto context_egl =
      std::make_unique<ContextEgl>(std::make_unique<EnvironmentEgl>(display_));

  native_window_ = std::make_unique<NativeWindowX11>(
      display_, context_egl->GetAttrib(EGL_NATIVE_VISUAL_ID), width, height);
  if (!native_window_->IsValid()) {
    ELINUX_LOG(ERROR) << "Failed to create the native window";
    return false;
  }

  render_surface_ = std::make_unique<SurfaceGl>(std::move(context_egl));
  render_surface_->SetNativeWindow(native_window_.get());

  return true;
}

void ELinuxWindowX11::DestroyRenderSurface() {
  // destroy the main surface before destroying the client window on X11.
  render_surface_ = nullptr;
  native_window_ = nullptr;
}

void ELinuxWindowX11::SetView(WindowBindingHandlerDelegate* window) {
  binding_handler_delegate_ = window;
}

ELinuxRenderSurfaceTarget* ELinuxWindowX11::GetRenderSurfaceTarget() const {
  return render_surface_.get();
}

double ELinuxWindowX11::GetDpiScale() { return current_scale_; }

PhysicalWindowBounds ELinuxWindowX11::GetPhysicalWindowBounds() {
  return {GetCurrentWidth(), GetCurrentHeight()};
}

int32_t ELinuxWindowX11::GetFrameRate() { return 60000; }

void ELinuxWindowX11::UpdateFlutterCursor(const std::string& cursor_name) {
  // TODO: implement here
}

void ELinuxWindowX11::UpdateVirtualKeyboardStatus(const bool show) {
  // currently not supported.
}

std::string ELinuxWindowX11::GetClipboardData() { return clipboard_data_; }

void ELinuxWindowX11::SetClipboardData(const std::string& data) {
  clipboard_data_ = data;
}

void ELinuxWindowX11::HandlePointerButtonEvent(uint32_t button,
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
        ELINUX_LOG(ERROR) << "Not expected button input: " << button;
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
