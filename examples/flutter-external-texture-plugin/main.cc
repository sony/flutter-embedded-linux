// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "command_options.h"
#include "external_texture_test_plugin.h"

int main(int argc, char** argv) {
  commandline::CommandOptions options;
  options.AddString("bundle", "b", "Path to Flutter app bundle", "./bundle",
                    true);
  options.AddWithoutValue("fullscreen", "f", "Always full-screen display",
                          false);
  options.AddWithoutValue("no-cursor", "n", "No mouse cursor/pointer", false);
  options.AddInt("width", "w", "Flutter app window width", 1280, false);
  options.AddInt("height", "h", "Flutter app window height", 720, false);
  if (!options.Parse(argc, argv)) {
    std::cerr << options.GetError() << std::endl;
    std::cout << options.ShowHelp();
    return 0;
  }

  // The project to run.
  const auto bundle_path = options.GetValue<std::string>("bundle");
  const std::wstring fl_path(bundle_path.begin(), bundle_path.end());
  flutter::DartProject project(fl_path);
  auto command_line_arguments = std::vector<std::string>();
  project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

  // The Flutter instance hosted by this window.
  const bool show_cursor = !options.Exist("no-cursor");
  const auto view_mode =
      options.Exist("fullscreen")
          ? flutter::FlutterViewController::ViewMode::kFullscreen
          : flutter::FlutterViewController::ViewMode::kNormal;
  const auto width = options.GetValue<int>("width");
  const auto height = options.GetValue<int>("height");
  auto flutter_controller = std::make_unique<flutter::FlutterViewController>(
      view_mode, width, height, show_cursor, project);

  // Ensure that basic setup of the controller was successful.
  if (!flutter_controller->engine() || !flutter_controller->view()) {
    return 0;
  }

  ExternalTextureTestPluginRegisterWithRegistrar(
      flutter_controller->engine()->GetRegistrarForPlugin(
          "ExternalTextureTestPlugin"));

  // Main loop.
  auto next_flutter_event_time =
      std::chrono::steady_clock::time_point::clock::now();
  while (flutter_controller->view()->DispatchEvent()) {
    // Wait until the next event.
    {
      auto wait_duration =
          std::max(std::chrono::nanoseconds(0),
                   next_flutter_event_time -
                       std::chrono::steady_clock::time_point::clock::now());
      std::this_thread::sleep_for(
          std::chrono::duration_cast<std::chrono::milliseconds>(wait_duration));
    }

    // Processes any pending events in the Flutter engine, and returns the
    // number of nanoseconds until the next scheduled event (or max, if none).
    auto wait_duration = flutter_controller->engine()->ProcessMessages();
    {
      auto next_event_time = std::chrono::steady_clock::time_point::max();
      if (wait_duration != std::chrono::nanoseconds::max()) {
        auto next_wakeup =
            std::max(std::chrono::steady_clock::time_point::max(),
                     std::chrono::steady_clock::time_point::clock::now() +
                         wait_duration);
        next_event_time = std::min(next_event_time, next_wakeup);
      } else {
        // Wait 1/60 [sec] = 13 [msec] if no events.
        next_event_time = std::chrono::steady_clock::time_point::clock::now() +
                          std::chrono::milliseconds(13);
      }
      next_flutter_event_time =
          std::max(next_flutter_event_time, next_event_time);
    }
  }

  return 0;
}
