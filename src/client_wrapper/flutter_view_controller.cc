// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/flutter_view_controller.h"

#include <algorithm>
#include <iostream>

namespace flutter {

FlutterViewController::FlutterViewController(
    const ViewProperties& view_properties,
    const DartProject& project) {
  engine_ = std::make_unique<FlutterEngine>(project);

  FlutterDesktopViewProperties c_view_properties = {};
  c_view_properties.width = view_properties.width;
  c_view_properties.height = view_properties.height;
  c_view_properties.view_rotation =
      (view_properties.view_rotation == ViewRotation::kRotation_90)
          ? FlutterDesktopViewRotation::kRotation_90
      : (view_properties.view_rotation == ViewRotation::kRotation_180)
          ? FlutterDesktopViewRotation::kRotation_180
      : (view_properties.view_rotation == ViewRotation::kRotation_270)
          ? FlutterDesktopViewRotation::kRotation_270
          : FlutterDesktopViewRotation::kRotation_0;
  c_view_properties.view_mode =
      (view_properties.view_mode == ViewMode::kFullscreen)
          ? FlutterDesktopViewMode::kFullscreen
          : FlutterDesktopViewMode::kNormalscreen;
  c_view_properties.title = view_properties.title.has_value()
                                ? (*view_properties.title).c_str()
                                : nullptr;
  c_view_properties.app_id = view_properties.app_id.has_value()
                                 ? (*view_properties.app_id).c_str()
                                 : nullptr;
  c_view_properties.use_mouse_cursor = view_properties.use_mouse_cursor;
  c_view_properties.use_onscreen_keyboard =
      view_properties.use_onscreen_keyboard;
  c_view_properties.use_window_decoration =
      view_properties.use_window_decoration;
  c_view_properties.text_scale_factor = view_properties.text_scale_factor;
  c_view_properties.enable_high_contrast = view_properties.enable_high_contrast;
  c_view_properties.force_scale_factor = view_properties.force_scale_factor;
  c_view_properties.scale_factor = view_properties.scale_factor;
  c_view_properties.enable_vsync = view_properties.enable_vsync;

  controller_ = FlutterDesktopViewControllerCreate(&c_view_properties,
                                                   engine_->RelinquishEngine());
  if (!controller_) {
    std::cerr << "Failed to create view controller." << std::endl;
    return;
  }

  view_ = std::make_unique<FlutterView>(
      FlutterDesktopViewControllerGetView(controller_));
}

FlutterViewController::~FlutterViewController() {
  if (controller_) {
    FlutterDesktopViewControllerDestroy(controller_);
  }
}

}  // namespace flutter
