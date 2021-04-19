// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"

#include <fcntl.h>
#include <xf86drm.h>

#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/cursor_data.h"

namespace flutter {

NativeWindowDrm::NativeWindowDrm(const char* device_filename) {
  drm_device_ = open(device_filename, O_RDWR | O_CLOEXEC);
  if (drm_device_ == -1) {
    LINUXES_LOG(ERROR) << "Couldn't open " << device_filename;
    return;
  }

  if (!ConfigureDisplay()) {
    return;
  }

  valid_ = true;
}

bool NativeWindowDrm::Resize(const size_t width, const size_t height) override {
  if (!valid_) {
    LINUXES_LOG(ERROR) << "Failed to resize the window.";
    return false;
  }

  // todo: implement here.
  LINUXES_LOG(ERROR) << "TODO: implement here!!";

  return false;
}

bool NativeWindowDrm::MoveCursor(double x, double y) {
  auto result =
      drmModeMoveCursor(drm_device_, drm_crtc_->crtc_id,
                        x - cursor_hotspot_.first, y - cursor_hotspot_.second);
  if (result < 0) {
    LINUXES_LOG(ERROR) << "Couldn't move the mouse cursor: " << result;
    return false;
  }
  return true;
}

bool NativeWindowDrm::ConfigureDisplay() {
  auto resources = drmModeGetResources(drm_device_);
  if (!resources) {
    LINUXES_LOG(ERROR) << "Couldn't get resources";
    return false;
  }

  auto connector = FindConnector(resources);
  if (!connector) {
    LINUXES_LOG(ERROR) << "Couldn't find any connectors";
    drmModeFreeResources(resources);
    return false;
  }

  drm_connector_id_ = connector->connector_id;
  drm_mode_info_ = connector->modes[0];
  width_ = drm_mode_info_.hdisplay;
  height_ = drm_mode_info_.vdisplay;
  LINUXES_LOG(INFO) << "resolution: " << width_ << "x" << height_;

  auto* encoder = FindEncoder(resources, connector);
  if (!encoder) {
    LINUXES_LOG(ERROR) << "Couldn't find any encoders";
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    return false;
  }
  if (encoder->crtc_id) {
    drm_crtc_ = drmModeGetCrtc(drm_device_, encoder->crtc_id);
  }

  drmModeFreeEncoder(encoder);
  drmModeFreeConnector(connector);
  drmModeFreeResources(resources);

  return true;
}

drmModeConnectorPtr NativeWindowDrm::FindConnector(drmModeResPtr resources) {
  for (int i = 0; i < resources->count_connectors; i++) {
    auto connector = drmModeGetConnector(drm_device_, resources->connectors[i]);
    // pick the first connected connector
    if (connector->connection == DRM_MODE_CONNECTED) {
      return connector;
    }
    drmModeFreeConnector(connector);
  }
  // no connector found
  return nullptr;
}

drmModeEncoder* NativeWindowDrm::FindEncoder(drmModeRes* resources,
                                             drmModeConnector* connector) {
  if (connector->encoder_id) {
    return drmModeGetEncoder(drm_device_, connector->encoder_id);
  }
  // no encoder found
  return nullptr;
}

const uint32_t* NativeWindowDrm::GetCursorData(const std::string& cursor_name) {
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

}  // namespace flutter
