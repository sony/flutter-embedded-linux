// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_H_

#include <libinput.h>
#include <systemd/sd-event.h>

#include <memory>
#include <thread>

#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_drm.h"
#include "flutter/shell/platform/linux_embedded/surface/native_window_drm.h"
#include "flutter/shell/platform/linux_embedded/window/linuxes_window.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"

namespace flutter {

class LinuxesWindowDrm : public LinuxesWindow, public WindowBindingHandler {
 public:
  LinuxesWindowDrm(FlutterWindowMode window_mode, int32_t width, int32_t height,
                   bool show_cursor);
  ~LinuxesWindowDrm();

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
  static void RunLibinputEventLoop(void* data);
  static int OnLibinputEvent(sd_event_source* source, int fd, uint32_t revents,
                             void* data);
  void ProcessLibinputEvent();
  void OnDeviceAdded(libinput_event* event);
  void OnDeviceRemoved(libinput_event* event);
  void OnKeyEvent(libinput_event* event);
  void OnPointerMotion(libinput_event* event);
  void OnPointerMotionAbsolute(libinput_event* event);
  void OnPointerButton(libinput_event* event);
  void OnPointerAxis(libinput_event* event);
  void OnTouchDown(libinput_event* event);
  void OnTouchUp(libinput_event* event);
  void OnTouchMotion(libinput_event* event);
  void OnTouchCancel(libinput_event* event);
  void ProcessPointerAxis(libinput_event_pointer* pointer_event,
                          libinput_pointer_axis axis);

  static const libinput_interface kLibinputInterface;

  // A pointer to a FlutterWindowsView that can be used to update engine
  // windowing and input state.
  WindowBindingHandlerDelegate* binding_handler_delegate_;

  std::unique_ptr<NativeWindowDrm> native_window_;
  std::unique_ptr<SurfaceGlDrm> render_surface_;

  bool display_valid_;
  bool is_pending_cursor_add_event_;

  sd_event* libinput_event_loop_;
  libinput* libinput_;
  std::thread thread_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_H_