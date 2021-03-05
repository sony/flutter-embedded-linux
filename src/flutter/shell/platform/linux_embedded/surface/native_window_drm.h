// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_NATIVE_WINDOW_DRM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_NATIVE_WINDOW_DRM_H_

#include <gbm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>

#include "flutter/shell/platform/linux_embedded/surface/native_window.h"

namespace flutter {

class NativeWindowDrm : public NativeWindow<gbm_surface, gbm_surface> {
 public:
  NativeWindowDrm(const char* deviceFilename);
  ~NativeWindowDrm();

  // |NativeWindow|
  bool Resize(const size_t width, const size_t height) const override;

  void SwapBuffer();

  gbm_device* BufferDevice() const { return gbm_device_; }

  int32_t Width() {
    if (!valid_) {
      return -1;
    }
    return drm_mode_info_.hdisplay;
  }

  int32_t Height() {
    if (!valid_) {
      return -1;
    }
    return drm_mode_info_.vdisplay;
  }

  bool ShowCursor(double x, double y);

  bool UpdateCursor(const std::string& cursor_name, double x, double y);

  bool MoveCursor(double x, double y);

  bool DismissCursor();

 private:
  bool ConfigureDisplay();

  drmModeConnectorPtr FindConnector(drmModeResPtr resources);

  drmModeEncoder* FindEncoder(drmModeRes* resources,
                              drmModeConnector* connector);

  bool CreateCursorBuffer(const std::string& cursor_name);

  // Convert Flutter's cursor value to cursor data.
  const uint32_t* GetCursorData(const std::string& cursor_name);

  int drm_device_;
  uint32_t drm_connector_id_;
  drmModeCrtc* drm_crtc_ = nullptr;
  drmModeModeInfo drm_mode_info_;

  gbm_bo* gbm_previous_bo_ = nullptr;
  uint32_t gbm_previous_fb_;
  gbm_device* gbm_device_ = nullptr;

  gbm_bo* gbm_cursor_bo_ = nullptr;
  std::string cursor_name_ = "";
  std::pair<int32_t, int32_t> cursor_hotspot_ = {0, 0};
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_NATIVE_WINDOW_DRM_H_