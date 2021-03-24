// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>

#include <iostream>
#include <memory>
#include <string>

static void PrintHelp() {
  std::cout << "Usage: ./${execute filename} {Flutter project bundle path}"
            << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    PrintHelp();
    return 0;
  }

  bool show_cursor = true;
  std::string str(argv[1]);
  std::wstring fl_path(str.begin(), str.end());

  // The project to run.
  flutter::DartProject project(fl_path);
  auto command_line_arguments = std::vector<std::string>();
  project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

  // The Flutter instance hosted by this window.
  int width = 1280;
  int height = 720;
  auto flutter_controller = std::make_unique<flutter::FlutterViewController>(
      flutter::FlutterViewController::ViewMode::kNormal, width, height,
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
