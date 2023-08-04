// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_PROJECT_BUNDLE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_PROJECT_BUNDLE_H_

#include <memory>
#include <string>
#include <vector>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux_embedded/public/flutter_elinux.h"

namespace flutter {

struct AotDataDeleter {
  void operator()(FlutterEngineAOTData aot_data) {
    FlutterEngineCollectAOTData(aot_data);
  }
};
using UniqueAotDataPtr = std::unique_ptr<_FlutterEngineAOTData, AotDataDeleter>;

// The data associated with a Flutter project needed to run it in an engine.
class FlutterProjectBundle {
 public:
  // Creates a new project bundle from the given properties.
  //
  // Treats any relative paths as relative to the directory of this executable.
  explicit FlutterProjectBundle(
      const FlutterDesktopEngineProperties& properties);

  ~FlutterProjectBundle() = default;

  // Whether or not the bundle is valid. This does not check that the paths
  // exist, or contain valid data, just that paths were able to be constructed.
  bool HasValidPaths();

  // Returns the path to the assets directory.
  const std::string& assets_path() { return assets_path_; }

  // Returns the path to the ICU data file.
  const std::string& icu_path() { return icu_path_; }

  // Returns any switches that should be passed to the engine.
  const std::vector<std::string> GetSwitches();

  // Sets engine switches.
  void SetSwitches(const std::vector<std::string>& switches);

  // Attempts to load AOT data for this bundle. The returned data must be
  // retained until any engine instance it is passed to has been shut down.
  //
  // Logs and returns nullptr on failure.
  UniqueAotDataPtr LoadAotData(const FlutterEngineProcTable& engine_procs);

  // Returns the command line arguments to be passed through to the Dart
  // entrypoint.
  const std::vector<std::string>& dart_entrypoint_arguments() const {
    return dart_entrypoint_arguments_;
  }

 private:
  // Returns the execuable directory path.
  const std::string GetExecutableDirectory();

  std::string assets_path_;
  std::string icu_path_;

  // Path to the AOT library file, if any.
  std::string aot_library_path_;

  // Dart entrypoint arguments.
  std::vector<std::string> dart_entrypoint_arguments_;

  // Engine switches.
  std::vector<std::string> engine_switches_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_PROJECT_BUNDLE_H_
