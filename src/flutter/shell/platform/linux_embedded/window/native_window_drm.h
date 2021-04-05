// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_H_

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>
#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/cursor_data.h"
#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

constexpr char kCursorNameNone[] = "none";

// Buffer size for cursor image. The size must be at least 64x64 due to the
// restrictions of drmModeSetCursor API.
constexpr uint32_t kCursorBufferWidth = 64;
constexpr uint32_t kCursorBufferHeight = 64;

template <typename T>
class NativeWindowDrm : public NativeWindow<T> {
 public:
  NativeWindowDrm() = default;
  virtual ~NativeWindowDrm() = default;

  // |NativeWindow|
  bool Resize(const size_t width, const size_t height) const override {
    if (!this->valid_) {
      LINUXES_LOG(ERROR) << "Failed to resize the window.";
      return false;
    }

    // todo: implement here.
    LINUXES_LOG(ERROR) << "TODO: implement here!!";

    return false;
  }

  virtual bool ShowCursor(double x, double y) = 0;

  virtual bool UpdateCursor(const std::string& cursor_name, double x,
                            double y) = 0;

  virtual bool DismissCursor() = 0;

  bool MoveCursor(double x, double y) {
    auto result = drmModeMoveCursor(drm_device_, drm_crtc_->crtc_id,
                                    x - cursor_hotspot_.first,
                                    y - cursor_hotspot_.second);
    if (result < 0) {
      LINUXES_LOG(ERROR) << "Could not move the mouse cursor: " << result;
      return false;
    }
    return true;
  }

  int32_t Width() {
    if (!this->valid_) {
      return -1;
    }
    return drm_mode_info_.hdisplay;
  }

  int32_t Height() {
    if (!this->valid_) {
      return -1;
    }
    return drm_mode_info_.vdisplay;
  }

  int* DrmDevice() { return &drm_device_; }

 protected:
  drmModeConnectorPtr FindConnector(drmModeResPtr resources) {
    for (int i = 0; i < resources->count_connectors; i++) {
      auto connector =
          drmModeGetConnector(drm_device_, resources->connectors[i]);
      // pick the first connected connector
      if (connector->connection == DRM_MODE_CONNECTED) {
        return connector;
      }
      drmModeFreeConnector(connector);
    }
    // no connector found
    return nullptr;
  }

  drmModeEncoder* FindEncoder(drmModeRes* resources,
                              drmModeConnector* connector) {
    if (connector->encoder_id) {
      return drmModeGetEncoder(drm_device_, connector->encoder_id);
    }
    // no encoder found
    return nullptr;
  }

  // Convert Flutter's cursor value to cursor data.
  const uint32_t* GetCursorData(const std::string& cursor_name) {
    // const uint32_t* NativeWindowDrm::GetCursorData(const std::string&
    // cursor_name) { If there is no cursor data corresponding to the Flutter's
    // cursor name, it is defined as empty. If empty, the default cursor
    // data(LeftPtr) will be displayed.
    static const std::unordered_map<std::string, const uint32_t*>
        flutter_to_drm_cursor_map = {
            {"alias", nullptr},
            {"allScroll", nullptr},
            {"basic", kCursorDataLeftPtr},
            {"cell", nullptr},
            {"click", kCursorDataHand1},
            {"contextMenu", nullptr},
            {"copy", nullptr},
            {"forbidden", nullptr},
            {"grab", nullptr},
            {"grabbing", kCursorDataGrabbing},
            {"help", nullptr},
            {"move", nullptr},
            {"noDrop", nullptr},
            {"precise", nullptr},
            {"progress", nullptr},
            {"text", kCursorDataXterm},
            {"resizeColumn", nullptr},
            {"resizeDown", kCursorDataBottomSide},
            {"resizeDownLeft", kCursorDataBottomLeftCorner},
            {"resizeDownRight", kCursorDataBottomRightCorner},
            {"resizeLeft", kCursorDataLeftSide},
            {"resizeLeftRight", nullptr},
            {"resizeRight", kCursorDataRightSide},
            {"resizeRow", nullptr},
            {"resizeUp", kCursorDataTopSide},
            {"resizeUpDown", nullptr},
            {"resizeUpLeft", kCursorDataTopLeftCorner},
            {"resizeUpRight", kCursorDataTopRightCorner},
            {"resizeUpLeftDownRight", nullptr},
            {"resizeUpRightDownLeft", nullptr},
            {"verticalText", nullptr},
            {"wait", kCursorDataWatch},
            {"zoomIn", nullptr},
            {"zoomOut", nullptr},
        };

    const uint32_t* cursor_data = nullptr;
    if (flutter_to_drm_cursor_map.find(cursor_name) !=
        flutter_to_drm_cursor_map.end()) {
      cursor_data = flutter_to_drm_cursor_map.at(cursor_name);
    }

    if (!cursor_data) {
      if (!cursor_name.empty()) {
        LINUXES_LOG(WARNING) << "Unsupported cursor: " << cursor_name.c_str()
                             << ", use LeftPtr cursor.";
      }
      cursor_data = kCursorDataLeftPtr;
    }

    cursor_hotspot_ = cursor_hotspot_map.at(cursor_data);
    return cursor_data;
  }

  int drm_device_;
  uint32_t drm_connector_id_;
  drmModeCrtc* drm_crtc_ = nullptr;
  drmModeModeInfo drm_mode_info_;

  std::string cursor_name_ = "";
  std::pair<int32_t, int32_t> cursor_hotspot_ = {0, 0};
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_H_