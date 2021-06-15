// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_H_

#include <string>
#include <variant>

#include "flutter/shell/platform/linux_embedded/public/flutter_linuxes.h"
#include "flutter/shell/platform/linux_embedded/surface/surface_gl.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler_delegate.h"

namespace flutter {

class FlutterWindowsView;

// Structure containing physical bounds of a Window
struct PhysicalWindowBounds {
  size_t width;
  size_t height;
};

using LinuxesRenderSurfaceTarget = SurfaceGl;

// Abstract class for binding Linux embedded platform windows to Flutter views.
class WindowBindingHandler {
 public:
  virtual ~WindowBindingHandler() = default;

  // Dispatches window events such as mouse and keyboard inputs. For Wayland,
  // you have to call this every time in the main loop.
  virtual bool DispatchEvent() = 0;

  // Create a surface.
  virtual bool CreateRenderSurface(int32_t width, int32_t height) = 0;

  // Destroy a surface which is currently used.
  virtual void DestroyRenderSurface() = 0;

  // Returns a valid LinuxesRenderSurfaceTarget representing the backing
  // window.
  virtual LinuxesRenderSurfaceTarget* GetRenderSurfaceTarget() const = 0;

  // Sets the delegate used to communicate state changes from window to view
  // such as key presses, mouse position updates etc.
  virtual void SetView(WindowBindingHandlerDelegate* view) = 0;

  // Returns the scale factor for the backing window.
  virtual double GetDpiScale() = 0;

  // Returns the bounds of the backing window in physical pixels.
  virtual PhysicalWindowBounds GetPhysicalWindowBounds() = 0;

  // Returns the frame rate of the display.
  virtual int32_t GetFrameRate() = 0;

  // Sets the cursor that should be used when the mouse is over the Flutter
  // content. See mouse_cursor.dart for the values and meanings of cursor_name.
  virtual void UpdateFlutterCursor(const std::string& cursor_name) = 0;

  // Sets the virtual keyboard status when the virtual keyboard needs to be
  // shown by Flutter events.
  virtual void UpdateVirtualKeyboardStatus(const bool show) = 0;

  // Returns the clipboard data.
  virtual std::string GetClipboardData() = 0;

  // Sets the clipboard data.
  virtual void SetClipboardData(const std::string& data) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_BINDING_HANDLER_H_
