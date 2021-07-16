// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_EMBEDDER_OPTIONS_
#define FLUTTER_EMBEDDER_OPTIONS_

#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>

#include <string>

#include "command_options.h"

class FlutterEmbedderOptions {
 public:
  FlutterEmbedderOptions() {
    options_.AddString("bundle", "b", "Path to Flutter app bundle", "./bundle",
                       true);
    options_.AddWithoutValue("no-cursor", "n", "No mouse cursor/pointer",
                             false);
#if defined(FLUTTER_TARGET_BACKEND_GBM) || \
    defined(FLUTTER_TARGET_BACKEND_EGLSTREAM)
    // no more options.
#elif defined(FLUTTER_TARGET_BACKEND_X11)
    options_.AddWithoutValue("fullscreen", "f", "Always full-screen display",
                             false);
    options_.AddInt("width", "w", "Flutter app window width", 1280, false);
    options_.AddInt("height", "h", "Flutter app window height", 720, false);
#else  // FLUTTER_TARGET_BACKEND_WAYLAND
    options_.AddWithoutValue("onscreen-keyboard", "k",
                             "Enable on-screen keyboard", false);
    options_.AddWithoutValue("window-decoration", "d",
                             "Enable window decorations", false);
    options_.AddWithoutValue("fullscreen", "f", "Always full-screen display",
                             false);
    options_.AddInt("width", "w", "Flutter app window width", 1280, false);
    options_.AddInt("height", "h", "Flutter app window height", 720, false);
#endif
  }
  ~FlutterEmbedderOptions() = default;

  bool Parse(int argc, char** argv) {
    if (!options_.Parse(argc, argv)) {
      std::cerr << options_.GetError() << std::endl;
      std::cout << options_.ShowHelp();
      return false;
    }

    bundle_path_ = options_.GetValue<std::string>("bundle");
    use_mouse_cursor_ = !options_.Exist("no-cursor");

#if defined(FLUTTER_TARGET_BACKEND_GBM) || \
    defined(FLUTTER_TARGET_BACKEND_EGLSTREAM)
    use_onscreen_keyboard_ = false;
    use_window_decoration_ = false;
    window_view_mode_ = flutter::FlutterViewController::ViewMode::kFullscreen;
#elif defined(FLUTTER_TARGET_BACKEND_X11)
    use_onscreen_keyboard_ = false;
    use_window_decoration_ = false;
    window_view_mode_ =
        options_.Exist("fullscreen")
            ? flutter::FlutterViewController::ViewMode::kFullscreen
            : flutter::FlutterViewController::ViewMode::kNormal;
    window_width_ = options_.GetValue<int>("width");
    window_height_ = options_.GetValue<int>("height");
#else  // FLUTTER_TARGET_BACKEND_WAYLAND
    use_onscreen_keyboard_ = options_.Exist("onscreen-keyboard");
    use_window_decoration_ = options_.Exist("window-decoration");
    window_view_mode_ =
        options_.Exist("fullscreen")
            ? flutter::FlutterViewController::ViewMode::kFullscreen
            : flutter::FlutterViewController::ViewMode::kNormal;
    window_width_ = options_.GetValue<int>("width");
    window_height_ = options_.GetValue<int>("height");
#endif

    return true;
  }

  std::string BundlePath() const { return bundle_path_; }
  bool IsUseMouseCursor() const { return use_mouse_cursor_; }
  bool IsUseOnscreenKeyboard() const { return use_onscreen_keyboard_; }
  bool IsUseWindowDecoraation() const { return use_window_decoration_; }
  flutter::FlutterViewController::ViewMode WindowViewMode() const {
    return window_view_mode_;
  }
  int WindowWidth() const { return window_width_; }
  int WindowHeight() const { return window_height_; }

 private:
  commandline::CommandOptions options_;

  std::string bundle_path_;
  bool use_mouse_cursor_ = true;
  bool use_onscreen_keyboard_ = false;
  bool use_window_decoration_ = false;
  flutter::FlutterViewController::ViewMode window_view_mode_ =
      flutter::FlutterViewController::ViewMode::kNormal;
  int window_width_ = 1280;
  int window_height_ = 720;
};

#endif  // FLUTTER_EMBEDDER_OPTIONS_
