// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PUBLIC_FLUTTER_PLATFORM_VIEWS_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PUBLIC_FLUTTER_PLATFORM_VIEWS_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "flutter_export.h"
#include "flutter_messenger.h"
#include "plugin_registrar.h"

class FlutterDesktopPlatformView {
 public:
  FlutterDesktopPlatformView(flutter::PluginRegistrar* registrar, int view_id)
      : registrar_(registrar),
        view_id_(view_id),
        texture_id_(-1),
        focused_(false) {}
  virtual ~FlutterDesktopPlatformView() {}

  flutter::PluginRegistrar* GetPluginRegistrar() { return registrar_; }

  virtual void Dispose() = 0;

  int GetViewId() const { return view_id_; }

  void SetFocus(bool focus) { focused_ = focus; }

  bool IsFocused() const { return focused_; }

  void SetTextureId(int texture_id) { texture_id_ = texture_id; }

  int GetTextureId() const { return texture_id_; }

 private:
  flutter::PluginRegistrar* registrar_;
  int view_id_;
  int texture_id_;
  bool focused_;
};

class FlutterDesktopPlatformViewFactory {
 public:
  FlutterDesktopPlatformViewFactory(flutter::PluginRegistrar* registrar)
      : registrar_(registrar) {}
  virtual ~FlutterDesktopPlatformViewFactory() {}

  flutter::PluginRegistrar* GetPluginRegistrar() const { return registrar_; }

  virtual FlutterDesktopPlatformView* Create(
      int view_id, double width, double height,
      const std::vector<uint8_t>& params) = 0;

  virtual void Dispose() = 0;

 private:
  flutter::PluginRegistrar* registrar_;
};

#if defined(__cplusplus)
extern "C" {
#endif

void FlutterDesktopRegisterPlatformViewFactory(
    FlutterDesktopPluginRegistrarRef registrar, const char* view_type,
    std::unique_ptr<FlutterDesktopPlatformViewFactory> view_factory)

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_PUBLIC_FLUTTER_PLATFORM_VIEWS_H_
