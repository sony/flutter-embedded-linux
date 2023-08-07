// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"

#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>

#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/cursor_data.h"

namespace flutter {

NativeWindowDrm::NativeWindowDrm(const char* device_filename,
                                 const uint16_t rotation,
                                 bool enable_vsync) {
  drm_device_ = open(device_filename, O_RDWR | O_CLOEXEC);
  if (drm_device_ == -1) {
    ELINUX_LOG(ERROR) << "Couldn't open " << device_filename;
    return;
  }

  if (!ConfigureDisplay(rotation)) {
    return;
  }

  enable_vsync_ = enable_vsync;
  valid_ = true;
}

NativeWindowDrm::~NativeWindowDrm() {
  if (drm_device_ != -1) {
    close(drm_device_);
  }
}

bool NativeWindowDrm::MoveCursor(double x, double y) {
  auto result =
      drmModeMoveCursor(drm_device_, drm_crtc_->crtc_id,
                        x - cursor_hotspot_.first, y - cursor_hotspot_.second);
  if (result < 0) {
    ELINUX_LOG(ERROR) << "Couldn't move the mouse cursor: " << result;
    return false;
  }
  return true;
}

bool NativeWindowDrm::ConfigureDisplay(const uint16_t rotation) {
  auto resources = drmModeGetResources(drm_device_);
  if (!resources) {
    ELINUX_LOG(ERROR) << "Couldn't get resources";
    return false;
  }

  auto connector = FindConnector(resources);
  if (!connector) {
    ELINUX_LOG(ERROR) << "Couldn't find any connectors";
    drmModeFreeResources(resources);
    return false;
  }

  drm_connector_id_ = connector->connector_id;
  drm_mode_info_ = connector->modes[0];
  width_ = drm_mode_info_.hdisplay;
  height_ = drm_mode_info_.vdisplay;
  if (rotation == 90 || rotation == 270) {
    std::swap(width_, height_);
  }
  ELINUX_LOG(INFO) << "resolution: " << width_ << "x" << height_;

  auto* encoder = FindEncoder(resources, connector);
  if (!encoder) {
    ELINUX_LOG(ERROR) << "Couldn't find any encoders";
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    return false;
  }
  if (!encoder->crtc_id) {
    // if there is no current CRTC, make sure to attach a suitable one
    for (int c = 0; c < resources->count_crtcs; c++) {
      if (encoder->possible_crtcs & (1 << c)) {
        encoder->crtc_id = resources->crtcs[c];
        break;
      }
    }
  }
  drm_crtc_ = drmModeGetCrtc(drm_device_, encoder->crtc_id);
  if (!drm_crtc_) {
    ELINUX_LOG(WARNING) << "Couldn't find a suitable crtc";
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
  drmModeEncoder* encoder = nullptr;
  // Find a suitable encoder
  for (int e = 0; e < resources->count_encoders; e++) {
    bool found = false;
    encoder = drmModeGetEncoder(drm_device_, resources->encoders[e]);
    for (int ce = 0; ce < connector->count_encoders; ce++) {
      if (encoder && encoder->encoder_id == connector->encoders[ce]) {
        ELINUX_LOG(DEBUG) << "Using encoder id " << encoder->encoder_id;
        found = true;
        break;
      }
    }
    if (found)
      break;
    drmModeFreeEncoder(encoder);
    encoder = nullptr;
  }

  // If encoder is not connected to the connector,
  // try to find a suitable one
  if (!encoder) {
    for (int e = 0; e < connector->count_encoders; e++) {
      encoder = drmModeGetEncoder(drm_device_, connector->encoders[e]);
      for (int c = 0; c < resources->count_crtcs; c++) {
        if (encoder->possible_crtcs & (1 << c)) {
          encoder->crtc_id = resources->crtcs[c];
          break;
        }
      }
      if (encoder->crtc_id)
        break;
      drmModeFreeEncoder(encoder);
      encoder = nullptr;
    }
  }

  // Will return nullptr if a suitable encoder is still not found
  return encoder;
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
      ELINUX_LOG(WARNING) << "Unsupported cursor: " << cursor_name.c_str()
                          << ", use LeftPtr cursor.";
    }
    cursor_data = kCursorDataLeftPtr;
  }

  cursor_hotspot_ = cursor_hotspot_map.at(cursor_data);
  return cursor_data;
}

}  // namespace flutter
