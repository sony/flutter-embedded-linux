// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_ENGINE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_ENGINE_H_

#include <rapidjson/document.h>

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "flutter/shell/platform/common/client_wrapper/binary_messenger_impl.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/basic_message_channel.h"
#include "flutter/shell/platform/common/incoming_message_dispatcher.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux_embedded/flutter_elinux_state.h"
#include "flutter/shell/platform/linux_embedded/flutter_elinux_texture_registrar.h"
#include "flutter/shell/platform/linux_embedded/flutter_project_bundle.h"
#include "flutter/shell/platform/linux_embedded/public/flutter_elinux.h"
#include "flutter/shell/platform/linux_embedded/task_runner.h"
#include "flutter/shell/platform/linux_embedded/vsync_waiter.h"

namespace flutter {

class FlutterELinuxView;

using FlutterDesktopMessengerReferenceOwner =
    std::unique_ptr<FlutterDesktopMessenger,
                    decltype(&FlutterDesktopMessengerRelease)>;

class FlutterELinuxEngine {
 public:
  explicit FlutterELinuxEngine(const FlutterProjectBundle& project);
  virtual ~FlutterELinuxEngine();

  // Prevent copying.
  FlutterELinuxEngine(FlutterELinuxEngine const&) = delete;
  FlutterELinuxEngine& operator=(FlutterELinuxEngine const&) = delete;

  // Starts running the engine with the given entrypoint. If null, defaults to
  // main().
  //
  // Returns false if the engine couldn't be started.
  bool RunWithEntrypoint(const char* entrypoint);

  // Returns true if the engine is currently running.
  bool running() { return engine_ != nullptr; }

  // Stops the engine. This invalidates the pointer returned by engine().
  //
  // Returns false if stopping the engine fails, or if it was not running.
  bool Stop();

  // Sets the view that is displaying this engine's content.
  void SetView(FlutterELinuxView* view);

  // The view displaying this engine's content, if any. This will be null for
  // headless engines.
  FlutterELinuxView* view() { return view_; }

  // Returns the currently configured Plugin Registrar.
  FlutterDesktopPluginRegistrarRef GetRegistrar();

  // Sets |callback| to be called when the plugin registrar is destroyed.
  void SetPluginRegistrarDestructionCallback(
      FlutterDesktopOnPluginRegistrarDestroyed callback);

  // Sets switches member to the given switches.
  void SetSwitches(const std::vector<std::string>& switches);

  FlutterDesktopMessengerRef messenger() SWIFT_RETURNS_UNRETAINED {
    return messenger_.get();
  }

  IncomingMessageDispatcher* message_dispatcher() {
    return message_dispatcher_.get();
  }

  TaskRunner* task_runner() { return task_runner_.get(); }

  FlutterELinuxTextureRegistrar* texture_registrar() {
    return texture_registrar_.get();
  }

  // Informs the engine that the window metrics have changed.
  void SendWindowMetricsEvent(const FlutterWindowMetricsEvent& event);

  // Informs the engine of an incoming pointer event.
  void SendPointerEvent(const FlutterPointerEvent& event);

  // Sends the given message to the engine, calling |reply| with |user_data|
  // when a reponse is received from the engine if they are non-null.
  bool SendPlatformMessage(const char* channel,
                           const uint8_t* message,
                           const size_t message_size,
                           const FlutterDesktopBinaryReply reply,
                           void* user_data);

  // Sends the given data as the response to an earlier platform message.
  void SendPlatformMessageResponse(
      const FlutterDesktopMessageResponseHandle* handle,
      const uint8_t* data,
      size_t data_length);

  // Callback passed to Flutter engine for notifying window of platform
  // messages.
  void HandlePlatformMessage(const FlutterPlatformMessage*);

  // Informs the engine that the system font list has changed.
  void ReloadSystemFonts();

  // Attempts to register the texture with the given |texture_id|.
  bool RegisterExternalTexture(int64_t texture_id);

  // Attempts to unregister the texture with the given |texture_id|.
  bool UnregisterExternalTexture(int64_t texture_id);

  // Notifies the engine about a new frame being available for the
  // given |texture_id|.
  bool MarkExternalTextureFrameAvailable(int64_t texture_id);

  // Posts the given callback onto the raster thread.
  bool PostRasterThreadTask(fml::closure callback);

  // Notifies the engine about the vsync event.
  void OnVsync(uint64_t last_frame_time_nanos,
               uint64_t vsync_interval_time_nanos);

  // Gets the status whether Impeller is enabled.
  bool IsImpellerEnabled() const { return enable_impeller_; }

  // Sets system settings.
  void SetSystemSettings(float text_scaling_factor, bool enable_high_contrast);

  // Updates accessibility, e.g. switch to high contrast mode
  void UpdateAccessibilityFeatures(FlutterAccessibilityFeature flags);

  // Update display information.
  void UpdateDisplayInfo(FlutterEngineDisplaysUpdateType update_type,
                         const FlutterEngineDisplay* displays,
                         size_t display_count);

 private:
  // Allows swapping out embedder_api_ calls in tests.
  friend class EngineEmbedderApiModifier;

  // Sends system locales to the engine.
  //
  // Should be called just after the engine is run, and after any relevant
  // system changes.
  void SendSystemLocales();

  // The handle to the embedder.h engine instance.
  FLUTTER_API_SYMBOL(FlutterEngine) engine_ = nullptr;

  FlutterEngineProcTable embedder_api_ = {};

  std::unique_ptr<FlutterProjectBundle> project_;

  // AOT data, if any.
  UniqueAotDataPtr aot_data_;

  // The view displaying the content running in this engine, if any.
  FlutterELinuxView* view_ = nullptr;

  // Task runner for tasks posted from the engine.
  std::unique_ptr<TaskRunner> task_runner_;

  // The plugin messenger handle given to API clients.
  FlutterDesktopMessengerReferenceOwner messenger_ = {
      nullptr, [](FlutterDesktopMessengerRef ref) {}};

  // A wrapper around messenger_ for interacting with client_wrapper-level APIs.
  std::unique_ptr<BinaryMessengerImpl> messenger_wrapper_;

  // Message dispatch manager for messages from engine_.
  std::unique_ptr<IncomingMessageDispatcher> message_dispatcher_;

  // The plugin registrar handle given to API clients.
  std::unique_ptr<FlutterDesktopPluginRegistrar> plugin_registrar_;

  // The texture registrar.
  std::unique_ptr<FlutterELinuxTextureRegistrar> texture_registrar_;

  // Resolved OpenGL functions used by external texture implementations.
  GlProcs gl_procs_ = {};

  // A callback to be called when the engine (and thus the plugin registrar)
  // is being destroyed.
  FlutterDesktopOnPluginRegistrarDestroyed
      plugin_registrar_destruction_callback_ = nullptr;

  // The vsync waiter.
  std::unique_ptr<VsyncWaiter> vsync_waiter_;

  bool enable_impeller_ = false;
} SWIFT_UNSAFE_REFERENCE;

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_ENGINE_H_
