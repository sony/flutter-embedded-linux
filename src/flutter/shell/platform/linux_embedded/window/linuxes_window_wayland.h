// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_WAYLAND_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_WAYLAND_H_

#include <wayland-client.h>
#include <wayland-cursor.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl.h"
#include "flutter/shell/platform/linux_embedded/window/linuxes_window.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_wayland.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"

extern "C" {
#include "wayland/protocol/text-input-unstable-v1-client-protocol.h"
#include "wayland/protocol/weston-desktop-shell-client-protocol.h"
#include "wayland/protocol/xdg-shell-client-protocol.h"
}

namespace flutter {

class LinuxesWindowWayland : public LinuxesWindow, public WindowBindingHandler {
 public:
  LinuxesWindowWayland(FlutterWindowMode window_mode, int32_t width,
                       int32_t height, bool show_cursor);
  ~LinuxesWindowWayland();

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
  struct CursorInfo {
    std::string cursor_name;
    uint32_t serial;
    wl_pointer* wl_pointer;
  };

  void WlRegistryHandler(wl_registry* wl_registry, uint32_t name,
                         const char* interface, uint32_t version);

  void WlUnRegistryHandler(wl_registry* wl_registry, uint32_t name);

  void CreateSupportedWlCursorList();

  wl_cursor* GetWlCursor(const std::string& cursor_name);

  static const wl_registry_listener kWlRegistryListener;
  static const xdg_wm_base_listener kXdgWmBaseListener;
  static const xdg_surface_listener kXdgSurfaceListener;
  static const wl_seat_listener kWlSeatListener;
  static const wl_pointer_listener kWlPointerListener;
  static const wl_touch_listener kWlTouchListener;
  static const wl_keyboard_listener kWlKeyboardListener;
  static const wl_output_listener kWlOutputListener;
  static const zwp_text_input_v1_listener kZwpTextInputV1Listener;
  static const wl_data_device_listener kWlDataDeviceListener;
  static const wl_data_source_listener kWlDataSourceListener;

  // A pointer to a FlutterWindowsView that can be used to update engine
  // windowing and input state.
  WindowBindingHandlerDelegate* binding_handler_delegate_;

  std::unique_ptr<NativeWindowWayland> native_window_;
  std::unique_ptr<SurfaceGl> render_surface_;

  bool display_valid_;

  wl_display* wl_display_;
  wl_registry* wl_registry_;
  wl_compositor* wl_compositor_;
  xdg_wm_base* xdg_wm_base_;
  xdg_surface* xdg_surface_;
  xdg_toplevel* xdg_toplevel_;
  wl_seat* wl_seat_;
  wl_output* wl_output_;
  wl_shm* wl_shm_;
  wl_pointer* wl_pointer_;
  wl_touch* wl_touch_;
  wl_keyboard* wl_keyboard_;
  wl_surface* wl_cursor_surface_;
  wl_cursor_theme* wl_cursor_theme_;
  zwp_text_input_manager_v1* zwp_text_input_manager_v1_;
  zwp_text_input_v1* zwp_text_input_v1_;

  CursorInfo cursor_info_;

  // List of cursor name and wl_cursor supported by Wayland.
  std::unordered_map<std::string, wl_cursor*> supported_wl_cursor_list_;

  wl_data_device_manager* wl_data_device_manager_;
  wl_data_device* wl_data_device_;
  wl_data_offer* wl_data_offer_;
  wl_data_source* wl_data_source_;
  uint32_t wl_data_device_manager_version_;
  uint32_t serial_;

  weston_desktop_shell* weston_desktop_shell_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_WAYLAND_H_