// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_X11_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_X11_H_

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_x11.h"
#include "flutter/shell/platform/linux_embedded/surface/native_window_x11.h"
#include "flutter/shell/platform/linux_embedded/window/linuxes_window.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"

namespace flutter {

class LinuxesWindowX11 : public LinuxesWindow, public WindowBindingHandler {
 public:
  LinuxesWindowX11(FlutterWindowMode window_mode, int32_t width, int32_t height,
                   bool show_cursor);
  ~LinuxesWindowX11();

  // |LinuxesWindow|
  bool IsValid() const override;

  // |FlutterWindowBindingHandler|
  bool DispatchEvent() override;

  // |FlutterWindowBindingHandler|
  bool CreateRenderSurface(int32_t width, int32_t height) override;

  // |FlutterWindowBindingHandler|
  void DestroyRenderSurface() override;

  // |FlutterWindowBindingHandler|
  void SetView(WindowBindingHandlerDelegate* view) override;

  // |FlutterWindowBindingHandler|
  LinuxesRenderSurfaceTarget* GetRenderSurfaceTarget() const override;

  // |FlutterWindowBindingHandler|
  double GetDpiScale() override;

  // |FlutterWindowBindingHandler|
  PhysicalWindowBounds GetPhysicalWindowBounds() override;

  // |FlutterWindowBindingHandler|
  void UpdateFlutterCursor(const std::string& cursor_name) override;

  // |FlutterWindowBindingHandler|
  void UpdateVirtualKeyboardStatus(const bool show) override;

  // |FlutterWindowBindingHandler|
  std::string GetClipboardData() override;

  // |FlutterWindowBindingHandler|
  void SetClipboardData(const std::string& data) override;

 private:
  // Handles the events of the mouse button.
  void HandlePointerButtonEvent(xcb_button_t button, bool button_pressed,
                                int16_t x, int16_t y);

  // A pointer to a FlutterWindowsView that can be used to update engine
  // windowing and input state.
  WindowBindingHandlerDelegate* binding_handler_delegate_;

  Display* display_ = nullptr;
  std::unique_ptr<NativeWindowX11> native_window_;
  std::unique_ptr<SurfaceGlX11> render_surface_;

  bool display_valid_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_X11_H_
