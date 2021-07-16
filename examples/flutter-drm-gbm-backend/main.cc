// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>

#include <iostream>
#include <memory>
#include <string>

#include "flutter_embedder_options.h"
#include "flutter_window.h"

int main(int argc, char** argv) {
  FlutterEmbedderOptions options;
  if (!options.Parse(argc, argv)) {
    return 0;
  }

  // Creates the Flutter project.
  const auto bundle_path = options.BundlePath();
  const std::wstring fl_path(bundle_path.begin(), bundle_path.end());
  flutter::DartProject project(fl_path);
  auto command_line_arguments = std::vector<std::string>();
  project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

  flutter::FlutterViewController::ViewProperties view_properties = {};
  view_properties.width = options.WindowWidth();
  view_properties.height = options.WindowHeight();
  view_properties.view_mode = options.WindowViewMode();
  view_properties.use_mouse_cursor = options.IsUseMouseCursor();
  view_properties.use_onscreen_keyboard = options.IsUseOnscreenKeyboard();
  view_properties.use_window_decoration = options.IsUseWindowDecoraation();

  // The Flutter instance hosted by this window.
  FlutterWindow window(view_properties, project);
  if (!window.OnCreate()) {
    std::cerr << "Failed to create a Flutter window." << std::endl;
    return 0;
  }
  window.Run();
  window.OnDestroy();

  return 0;
}
