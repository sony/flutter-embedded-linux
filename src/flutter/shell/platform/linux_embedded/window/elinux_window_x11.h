// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_X11_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_X11_H_

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/surface_gl.h"
#include "flutter/shell/platform/linux_embedded/window/elinux_window.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_x11.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"

namespace flutter {

class ELinuxWindowX11 : public ELinuxWindow, public WindowBindingHandler {
 public:
  ELinuxWindowX11(FlutterDesktopViewProperties view_properties);
  ~ELinuxWindowX11();

  // |ELinuxWindow|
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
  ELinuxRenderSurfaceTarget* GetRenderSurfaceTarget() const override;

  // |FlutterWindowBindingHandler|
  double GetDpiScale() override;

  // |FlutterWindowBindingHandler|
  PhysicalWindowBounds GetPhysicalWindowBounds() override;

  // |FlutterWindowBindingHandler|
  int32_t GetFrameRate() override;

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
  void HandlePointerButtonEvent(uint32_t button,
                                bool button_pressed,
                                int16_t x,
                                int16_t y);

  // A pointer to a FlutterWindowsView that can be used to update engine
  // windowing and input state.
  WindowBindingHandlerDelegate* binding_handler_delegate_ = nullptr;

  Display* display_ = nullptr;
  std::unique_ptr<NativeWindowX11> native_window_;
  std::unique_ptr<SurfaceGl> render_surface_;

  bool display_valid_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_X11_H_
