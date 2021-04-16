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
