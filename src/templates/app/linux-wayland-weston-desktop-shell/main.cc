// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <config-parser.h>
#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>

#include <iostream>
#include <memory>
#include <string>

int main(int argc, char** argv) {
  // Works as a weston desktop shell.
  int show_cursor = 1;
  auto config_file = weston_config_get_name_from_env();
  auto config = weston_config_parse(config_file);
  auto s =
      weston_config_get_section(config, "flutter_linux_wayland", NULL, NULL);
  char* path = nullptr;
  if (s) {
    weston_config_section_get_bool(s, "show-cursor", &show_cursor, 1);
    weston_config_section_get_string(
        s, "flutter-project-path", &path,
        "../../sample-app/sample/build/linux/x64/release/bundle");
  }

  std::wstring fl_path;
  if (path != nullptr) {
    // convert char* to wstring.
    std::string str(path);
    std::wstring ws_temp(str.begin(), str.end());
    if (!ws_temp.empty()) {
      fl_path = ws_temp;
    }

    free(path);
  }

  // libweston-6 does not export weston_config_destroy().
  // So run free() instead.
  if (config) {
    free(config);
  }
  // weston_config_destroy(config);

  // The project to run.
  flutter::DartProject project(fl_path);
  auto command_line_arguments = std::vector<std::string>();
  project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

  // The Flutter instance hosted by this window.
  int width = 640;
  int height = 480;
  auto flutter_controller = std::make_unique<flutter::FlutterViewController>(
      flutter::FlutterViewController::ViewMode::kFullscreen, width, height,
      show_cursor, project);

  // Ensure that basic setup of the controller was successful.
  if (!flutter_controller->engine() || !flutter_controller->view()) {
    return 0;
  }

  // Main loop.
  while (flutter_controller->view()->DispatchEvent()) {
    flutter_controller->engine()->ProcessMessages();
  }

  return 0;
}
