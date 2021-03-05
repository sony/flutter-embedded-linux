// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/flutter_project_bundle.h"

#include <unistd.h>

#include <iostream>
#include <string>

#include "flutter/shell/platform/common/engine_switches.h"
#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {

std::string ConvertWcharToString(const wchar_t* wchar) {
  std::wstring ws(wchar);
  std::string str(ws.begin(), ws.end());
  return str;
}

}  // namespace

FlutterProjectBundle::FlutterProjectBundle(
    const FlutterDesktopEngineProperties& properties) {
  assets_path_ = ConvertWcharToString(properties.assets_path);
  icu_path_ = ConvertWcharToString(properties.icu_data_path);
  if (properties.aot_library_path != nullptr) {
    aot_library_path_ = ConvertWcharToString(properties.aot_library_path);
  } else {
    aot_library_path_ = "";
  }

  for (int i = 0; i < properties.dart_entrypoint_argc; i++) {
    dart_entrypoint_arguments_.push_back(
        std::string(properties.dart_entrypoint_argv[i]));
  }

  // Resolve any relative paths.
  std::string project_path;
  if (assets_path_.compare(0, 1, "/") != 0 ||
      icu_path_.compare(0, 1, "/") != 0 ||
      (!aot_library_path_.empty() &&
       aot_library_path_.compare(0, 1, "/") != 0)) {
    auto executable_location = GetExecutableDirectory();
    if (executable_location.empty()) {
      LINUXES_LOG(ERROR)
          << "Unable to find executable location to resolve resource paths.";
    } else {
      assets_path_ = executable_location + "/" + assets_path_;
      icu_path_ = executable_location + "/" + icu_path_;
      if (!aot_library_path_.empty()) {
        aot_library_path_ = executable_location + "/" + aot_library_path_;
      }
    }
  }
}

bool FlutterProjectBundle::HasValidPaths() {
  return !assets_path_.empty() && !icu_path_.empty();
}

// Attempts to load AOT data from the given path, which must be absolute and
// non-empty. Logs and returns nullptr on failure.
UniqueAotDataPtr FlutterProjectBundle::LoadAotData(
    const FlutterEngineProcTable& engine_procs) {
  if (aot_library_path_.empty()) {
    LINUXES_LOG(ERROR)
        << "Attempted to load AOT data, but no aot_library_path was provided.";
    return nullptr;
  }

  FlutterEngineAOTDataSource source = {};
  source.type = kFlutterEngineAOTDataSourceTypeElfPath;
  source.elf_path = aot_library_path_.c_str();
  FlutterEngineAOTData data = nullptr;
  auto result = engine_procs.CreateAOTData(&source, &data);
  if (result != kSuccess) {
    LINUXES_LOG(ERROR) << "Failed to load AOT data from: " << aot_library_path_;
    return nullptr;
  }
  return UniqueAotDataPtr(data);
}

const std::vector<std::string> FlutterProjectBundle::GetSwitches() {
  return GetSwitchesFromEnvironment();
}

const std::string FlutterProjectBundle::GetExecutableDirectory() {
  static char buf[1024] = {};
  readlink("/proc/self/exe", buf, sizeof(buf) - 1);

  std::string exe_path = std::string(buf);
  const int slash_pos = exe_path.find_last_of('/');
  return exe_path.substr(0, slash_pos);
}

}  // namespace flutter
