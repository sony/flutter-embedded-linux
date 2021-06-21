// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PUBLIC_FLUTTER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PUBLIC_FLUTTER_H_

#include <stddef.h>
#include <stdint.h>

#include "flutter_export.h"
#include "flutter_messenger.h"
#include "flutter_plugin_registrar.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Opaque reference to a Flutter window controller.
typedef struct FlutterDesktopViewControllerState*
    FlutterDesktopViewControllerRef;

// Opaque reference to a Flutter window.
struct FlutterDesktopView;
typedef struct FlutterDesktopView* FlutterDesktopViewRef;

// Opaque reference to a Flutter engine instance.
struct FlutterDesktopEngine;
typedef struct FlutterDesktopEngine* FlutterDesktopEngineRef;

// Properties for configuring a Flutter engine instance.
typedef struct {
  // The path to the flutter_assets folder for the application to be run.
  // This can either be an absolute path or a path relative to the directory
  // containing the executable.
  const wchar_t* assets_path;

  // The path to the icudtl.dat file for the version of Flutter you are using.
  // This can either be an absolute path or a path relative to the directory
  // containing the executable.
  const wchar_t* icu_data_path;

  // The path to the AOT libary file for your application, if any.
  // This can either be an absolute path or a path relative to the directory
  // containing the executable. This can be nullptr for a non-AOT build, as
  // it will be ignored in that case.
  const wchar_t* aot_library_path;

  // Number of elements in the array passed in as dart_entrypoint_argv.
  int dart_entrypoint_argc;

  // Array of Dart entrypoint arguments. This is deep copied during the call
  // to FlutterDesktopEngineCreate.
  const char** dart_entrypoint_argv;
} FlutterDesktopEngineProperties;

// The View display mode.
enum FlutterDesktopViewMode {
  // Shows the Flutter view by user specific size.
  kNormal = 0,
  // Shows always the Flutter view by fullscreen.
  kFullscreen = 1,
};

// Properties for configuring a Flutter view instance.
typedef struct {
  // View width.
  int width;

  // View height.
  int height;

  // View display mode. If you set kFullscreen, the parameters of both `width`
  // and `height` will be ignored.
  FlutterDesktopViewMode view_mode;

  // Uses mouse cursor.
  bool use_mouse_cursor;

  // Uses the on-screen keyboard.
  bool use_onscreen_keyboard;

  // Uses the window decoration such as toolbar and max/min buttons.
  // This option is only active for Wayland backend.
  bool use_window_decoration;
} FlutterDesktopViewProperties;

// ========== View Controller ==========

// Creates a view that hosts and displays the given engine instance.
//
// This takes ownership of |engine|, so FlutterDesktopEngineDestroy should no
// longer be called on it, as it will be called internally when the view
// controller is destroyed. If creating the view controller fails, the engine
// will be destroyed immediately.
//
// If |engine| is not already running, the view controller will start running
// it automatically before displaying the window.
//
// The caller owns the returned reference, and is responsible for calling
// FlutterDesktopViewControllerDestroy. Returns a null pointer in the event of
// an error.
FLUTTER_EXPORT FlutterDesktopViewControllerRef
FlutterDesktopViewControllerCreate(
    const FlutterDesktopViewProperties& view_properties,
    FlutterDesktopEngineRef engine);

// Shuts down the engine instance associated with |controller|, and cleans up
// associated state.
//
// |controller| is no longer valid after this call.
FLUTTER_EXPORT void FlutterDesktopViewControllerDestroy(
    FlutterDesktopViewControllerRef controller);

// Returns the handle for the engine running in FlutterDesktopViewControllerRef.
//
// Its lifetime is the same as the |controller|'s.
FLUTTER_EXPORT FlutterDesktopEngineRef FlutterDesktopViewControllerGetEngine(
    FlutterDesktopViewControllerRef controller);

// Returns the view managed by the given controller.
FLUTTER_EXPORT FlutterDesktopViewRef
FlutterDesktopViewControllerGetView(FlutterDesktopViewControllerRef controller);

FLUTTER_EXPORT bool FlutterDesktopViewDispatchEvent(FlutterDesktopViewRef view);

// Returns the display frame rate by the given controller.
FLUTTER_EXPORT int32_t
FlutterDesktopViewGetFrameRate(FlutterDesktopViewRef view);

// ========== Engine ==========

// Creates a Flutter engine with the given properties.
//
// The caller owns the returned reference, and is responsible for calling
// FlutterDesktopEngineDestroy.
FLUTTER_EXPORT FlutterDesktopEngineRef FlutterDesktopEngineCreate(
    const FlutterDesktopEngineProperties& engine_properties);

// Shuts down and destroys the given engine instance. Returns true if the
// shutdown was successful, or if the engine was not running.
//
// |engine| is no longer valid after this call.
FLUTTER_EXPORT bool FlutterDesktopEngineDestroy(FlutterDesktopEngineRef engine);

// Starts running the given engine instance and optional entry point in the Dart
// project. If the entry point is null, defaults to main().
//
// If provided, entry_point must be the name of a top-level function from the
// same Dart library that contains the app's main() function, and must be
// decorated with `@pragma(vm:entry-point)` to ensure the method is not
// tree-shaken by the Dart compiler.
//
// Returns false if running the engine failed.
FLUTTER_EXPORT bool FlutterDesktopEngineRun(FlutterDesktopEngineRef engine,
                                            const char* entry_point);

// Processes any pending events in the Flutter engine, and returns the
// number of nanoseconds until the next scheduled event (or max, if none).
//
// This should be called on every run of the application-level runloop, and
// a wait for native events in the runloop should never be longer than the
// last return value from this function.
FLUTTER_EXPORT uint64_t
FlutterDesktopEngineProcessMessages(FlutterDesktopEngineRef engine);

FLUTTER_EXPORT void FlutterDesktopEngineReloadSystemFonts(
    FlutterDesktopEngineRef engine);

// Returns the plugin registrar handle for the plugin with the given name.
//
// The name must be unique across the application.
FLUTTER_EXPORT FlutterDesktopPluginRegistrarRef
FlutterDesktopEngineGetPluginRegistrar(FlutterDesktopEngineRef engine,
                                       const char* plugin_name);

// Returns the messenger associated with the engine.
FLUTTER_EXPORT FlutterDesktopMessengerRef
FlutterDesktopEngineGetMessenger(FlutterDesktopEngineRef engine);

// Returns the texture registrar associated with the engine.
FLUTTER_EXPORT FlutterDesktopTextureRegistrarRef
FlutterDesktopEngineGetTextureRegistrar(
    FlutterDesktopTextureRegistrarRef texture_registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PUBLIC_FLUTTER_H_
