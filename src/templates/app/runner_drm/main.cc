// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>

#include <iostream>
#include <memory>
#include <string>

#include "command_options.h"
#include "flutter_window.h"

int main(int argc, char** argv) {
  commandline::CommandOptions options;
  options.AddString("bundle", "b", "Path to Flutter app bundle", "./bundle",
                    true);
  options.AddWithoutValue("no-cursor", "n", "No mouse cursor/pointer", false);
  if (!options.Parse(argc, argv)) {
    std::cerr << options.GetError() << std::endl;
    std::cout << options.ShowHelp();
    return 0;
  }

  // The project to run.
  const bool show_cursor = !options.Exist("no-cursor");
  const auto bundle_path = options.GetValue<std::string>("bundle");

  const std::wstring fl_path(bundle_path.begin(), bundle_path.end());
  flutter::DartProject project(fl_path);
  auto command_line_arguments = std::vector<std::string>();
  project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

  flutter::FlutterViewController::ViewProperties view_properties = {};
  view_properties.view_mode =
      flutter::FlutterViewController::ViewMode::kFullscreen;
  view_properties.use_mouse_cursor = show_cursor;
  view_properties.use_onscreen_keyboard = false;
  view_properties.use_window_decoration = false;

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
