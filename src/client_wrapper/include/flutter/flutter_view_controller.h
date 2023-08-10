// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_CONTROLLER_H_

#include <flutter_elinux.h>

#include <memory>
#include <optional>
#include <string>

#include "dart_project.h"
#include "flutter_engine.h"
#include "flutter_view.h"
#include "plugin_registrar.h"
#include "plugin_registry.h"

namespace flutter {

// A controller for a view displaying Flutter content.
//
// This is the primary wrapper class for the desktop C API.
// If you use this class, you should not call any of the setup or teardown
// methods in the C API directly, as this class will do that internally.
class FlutterViewController {
 public:
  enum ViewMode {
    // Shows the Flutter view by user specific size.
    kNormal = 0,
    // Shows always the Flutter view by fullscreen.
    kFullscreen = 1,
  };

  enum ViewRotation {
    // Rotation constant: 0 degree rotation (natural orientation)
    kRotation_0 = 0,
    // Rotation constant: 90 degree rotation.
    kRotation_90 = 1,
    // Rotation constant: 180 degree rotation.
    kRotation_180 = 2,
    // Rotation constant: 270 degree rotation.
    kRotation_270 = 3,
  };

  // Properties for configuring a Flutter view instance.
  typedef struct {
    // View width.
    int width;

    // View height.
    int height;

    // View rotation.
    ViewRotation view_rotation;

    // View display mode. If you set kFullscreen, the parameters of both `width`
    // and `height` will be ignored.
    ViewMode view_mode;

    // View title.
    std::optional<std::string> title;

    // View XDG application ID. As a best practice, it is suggested to select an
    // app ID that matches the basename of the application's .desktop file.
    std::optional<std::string> app_id;

    // Uses mouse cursor.
    bool use_mouse_cursor;

    // Uses the on-screen keyboard.
    bool use_onscreen_keyboard;

    // Uses the window decoration such as toolbar and max/min buttons.
    // This option is only active for Wayland backend.
    bool use_window_decoration;

    // Text scaling factor.
    double text_scale_factor;

    // Force scale factor specified by command line argument
    bool force_scale_factor;
    double scale_factor;

    // Enable Vsync.
    // True:  Sync to compositor redraw/v-blank  (eglSwapInterval 1)
    // False: Do not sync to compositor redraw/v-blank (eglSwapInterval 0)
    bool enable_vsync;
  } ViewProperties;

  // Creates a FlutterView that can be parented into a Windows View hierarchy
  // either using HWNDs or in the future into a CoreWindow, or using compositor.
  //
  // |dart_project| will be used to configure the engine backing this view.
  explicit FlutterViewController(const ViewProperties& view_properties,
                                 const DartProject& project);

  virtual ~FlutterViewController();

  // Prevent copying.
  FlutterViewController(FlutterViewController const&) = delete;
  FlutterViewController& operator=(FlutterViewController const&) = delete;

  // Returns the engine running Flutter content in this view.
  FlutterEngine* engine() { return engine_.get(); }

  // Returns the view managed by this controller.
  FlutterView* view() { return view_.get(); }

 private:
  // Handle for interacting with the C API's view controller, if any.
  FlutterDesktopViewControllerRef controller_ = nullptr;

  // The backing engine
  std::unique_ptr<FlutterEngine> engine_;

  // The owned FlutterView.
  std::unique_ptr<FlutterView> view_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_CONTROLLER_H_
