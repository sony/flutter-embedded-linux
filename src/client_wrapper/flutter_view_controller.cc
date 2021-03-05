// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/flutter_view_controller.h"

#include <algorithm>
#include <iostream>

namespace flutter {

FlutterViewController::FlutterViewController(ViewMode view_mode, int width,
                                             int height, bool show_cursor,
                                             const DartProject& project) {
  engine_ = std::make_unique<FlutterEngine>(project);

  FlutterDesktopViewProperties c_view_properties = {};
  c_view_properties.width = width;
  c_view_properties.height = height;
  c_view_properties.windw_display_mode = (view_mode == ViewMode::kFullscreen)
                                             ? FlutterWindowMode::kFullscreen
                                             : FlutterWindowMode::kNormal;
  c_view_properties.show_cursor = show_cursor;
  controller_ = FlutterDesktopViewControllerCreate(c_view_properties,
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
