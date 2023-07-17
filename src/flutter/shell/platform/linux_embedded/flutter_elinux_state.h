// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_STATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_STATE_H_

#include <atomic>
#include <memory>
#include <mutex>

#if __has_include(<swift/bridging>)
#include <swift/bridging>
#else
#define SWIFT_SHARED_REFERENCE(_retain, _release)
#endif

#include "flutter/shell/platform/common/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/incoming_message_dispatcher.h"
#include "flutter/shell/platform/embedder/embedder.h"

// Structs backing the opaque references used in the C API.
//
// DO NOT ADD ANY NEW CODE HERE. These are legacy, and are being phased out
// in favor of objects that own and manage the relevant functionality.

namespace flutter {
struct FlutterELinuxEngine;
struct FlutterELinuxView;
}  // namespace flutter

// Wrapper to distinguish the view controller ref from the view ref given out
// in the C API.
struct FlutterDesktopViewControllerState {
  // The view that backs this state object.
  std::unique_ptr<flutter::FlutterELinuxView> view;
};

// Wrapper to distinguish the plugin registrar ref from the engine ref given out
// in the C API.
struct FlutterDesktopPluginRegistrar {
  // The engine that owns this state object.
  flutter::FlutterELinuxEngine* engine = nullptr;
};

// Wrapper to distinguish the messenger ref from the engine ref given out
// in the C API.
struct FlutterDesktopMessenger {
  FlutterDesktopMessenger() = default;

  /// Increments the reference count.
  ///
  /// Thread-safe.
  void AddRef() { ref_count_.fetch_add(1); }

  /// Decrements the reference count and deletes the object if the count has
  /// gone to zero.
  ///
  /// Thread-safe.
  void Release() {
    int32_t old_count = ref_count_.fetch_sub(1);
    if (old_count <= 1) {
      delete this;
    }
  }

  /// Getter for the engine field.
  flutter::FlutterELinuxEngine* GetEngine() const { return engine_; }

  /// Setter for the engine field.
  /// Thread-safe.
  void SetEngine(flutter::FlutterELinuxEngine* engine) {
    std::scoped_lock lock(mutex_);
    engine_ = engine;
  }

  /// Returns the mutex associated with the |FlutterDesktopMessenger|.
  ///
  /// This mutex is used to synchronize reading or writing state inside the
  /// |FlutterDesktopMessenger| (ie |engine_|).
  std::mutex& GetMutex() { return mutex_; }

  FlutterDesktopMessenger(const FlutterDesktopMessenger& value) = delete;
  FlutterDesktopMessenger& operator=(const FlutterDesktopMessenger& value) =
      delete;

 private:
  // The engine that backs this messenger.
  flutter::FlutterELinuxEngine* engine_;
  std::atomic<int32_t> ref_count_ = 0;
  std::mutex mutex_;
} SWIFT_SHARED_REFERENCE(FlutterDesktopMessengerAddRef,
                         FlutterDesktopMessengerRelease);

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_STATE_H_
