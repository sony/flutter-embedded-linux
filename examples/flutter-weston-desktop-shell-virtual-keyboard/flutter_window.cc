// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_window.h"

#include <chrono>
#include <thread>

#include "generated_plugin_registrant.h"

FlutterWindow::FlutterWindow(const flutter::DartProject project)
    : project_(project) {}

bool FlutterWindow::OnCreate(flutter::FlutterViewController::ViewMode view_mode,
                             int width, int height, bool show_cursor) {
  flutter_view_controller_ = std::make_unique<flutter::FlutterViewController>(
      view_mode, width, height, show_cursor, project_);

  // Ensure that basic setup of the controller was successful.
  if (!flutter_view_controller_->engine() ||
      !flutter_view_controller_->view()) {
    return false;
  }

  // Register Flutter plugins.
  RegisterPlugins(flutter_view_controller_->engine());

  return true;
}

void FlutterWindow::OnDestroy() {
  if (flutter_view_controller_) {
    flutter_view_controller_ = nullptr;
  }
}

void FlutterWindow::Run() {
  // Main loop.
  auto next_flutter_event_time =
      std::chrono::steady_clock::time_point::clock::now();
  while (flutter_view_controller_->view()->DispatchEvent()) {
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
    auto wait_duration = flutter_view_controller_->engine()->ProcessMessages();
    {
      auto next_event_time = std::chrono::steady_clock::time_point::max();
      if (wait_duration != std::chrono::nanoseconds::max()) {
        next_event_time =
            std::min(next_event_time,
                     std::chrono::steady_clock::time_point::clock::now() +
                         wait_duration);
      } else {
        // Wait 1/60 [sec] = 13 [msec] if no events.
        next_event_time =
            std::min(next_event_time,
                     std::chrono::steady_clock::time_point::clock::now() +
                         std::chrono::milliseconds(13));
      }
      next_flutter_event_time =
          std::max(next_flutter_event_time, next_event_time);
    }
  }
}
