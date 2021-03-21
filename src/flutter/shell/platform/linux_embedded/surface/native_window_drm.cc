// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/native_window_drm.h"

#include <fcntl.h>
#include <unistd.h>

#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/cursor_data.h"

namespace flutter {

static constexpr char kCursorNameNone[] = "none";

// Buffer size for cursor image. The size must be at least 64x64 due to the
// restrictions of drmModeSetCursor API.
static constexpr uint32_t kCursorBufferWidth = 64;
static constexpr uint32_t kCursorBufferHeight = 64;

NativeWindowDrm::NativeWindowDrm(const char* deviceFilename) {
  drm_device_ = open(deviceFilename, O_RDWR | O_CLOEXEC);
  if (drm_device_ == -1) {
    LINUXES_LOG(ERROR) << "Couldn't open " << deviceFilename;
    return;
  }

  if (!drmIsMaster(drm_device_)) {
    LINUXES_LOG(ERROR)
        << "Couldn't become the DRM master. Please confirm if another display "
           "backend such as X11 and Wayland is not running.";
    return;
  }

  ConfigureDisplay();

  gbm_device_ = gbm_create_device(drm_device_);
  if (!gbm_device_) {
    LINUXES_LOG(ERROR) << "Couldn't create the GBM device.";
    return;
  }

  window_ = gbm_surface_create(gbm_device_, drm_mode_info_.hdisplay,
                               drm_mode_info_.vdisplay, GBM_BO_FORMAT_ARGB8888,
                               GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
  if (!window_) {
    LINUXES_LOG(ERROR) << "Failed to create the gbm surface.";
    return;
  }

  valid_ = true;
}

NativeWindowDrm::~NativeWindowDrm() {
  if (drm_device_ == -1) {
    return;
  }

  if (gbm_cursor_bo_) {
    gbm_bo_destroy(gbm_cursor_bo_);
    gbm_cursor_bo_ = nullptr;
  }

  if (drm_crtc_) {
    drmModeSetCrtc(drm_device_, drm_crtc_->crtc_id, drm_crtc_->buffer_id,
                   drm_crtc_->x, drm_crtc_->y, &drm_connector_id_, 1,
                   &drm_crtc_->mode);
    drmModeFreeCrtc(drm_crtc_);
  }

  if (gbm_previous_bo_) {
    drmModeRmFB(drm_device_, gbm_previous_fb_);
    gbm_surface_release_buffer(window_, gbm_previous_bo_);
    gbm_surface_destroy(window_);
    window_ = nullptr;
  }

  if (gbm_device_) {
    gbm_device_destroy(gbm_device_);
  }

  close(drm_device_);
}

bool NativeWindowDrm::Resize(const size_t width, const size_t height) const {
  if (!valid_) {
    LINUXES_LOG(ERROR) << "Failed to resize the window.";
    return false;
  }

  // todo: implement here.
  LINUXES_LOG(ERROR) << "TODO: implement here!!";

  return false;
}

void NativeWindowDrm::SwapBuffer() {
  auto* bo = gbm_surface_lock_front_buffer(window_);
  auto width = gbm_bo_get_width(bo);
  auto height = gbm_bo_get_height(bo);
  auto handle = gbm_bo_get_handle(bo).u32;
  auto stride = gbm_bo_get_stride(bo);
  uint32_t fb;
  int result =
      drmModeAddFB(drm_device_, width, height, 24, 32, stride, handle, &fb);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to add a framebuffer. (" << result << ")";
  }
  result = drmModeSetCrtc(drm_device_, drm_crtc_->crtc_id, fb, 0, 0,
                          &drm_connector_id_, 1, &drm_mode_info_);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to set crct mode. (" << result << ")";
  }

  if (gbm_previous_bo_) {
    drmModeRmFB(drm_device_, gbm_previous_fb_);
    gbm_surface_release_buffer(window_, gbm_previous_bo_);
  }
  gbm_previous_bo_ = bo;
  gbm_previous_fb_ = fb;
}

bool NativeWindowDrm::ShowCursor(double x, double y) {
  if (!gbm_cursor_bo_ && !CreateCursorBuffer(cursor_name_)) {
    return false;
  }

  MoveCursor(x, y);
  auto result = drmModeSetCursor(drm_device_, drm_crtc_->crtc_id,
                                 gbm_bo_get_handle(gbm_cursor_bo_).u32,
                                 kCursorBufferWidth, kCursorBufferHeight);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to set cursor buffer. (" << result << ")";
    return false;
  }
  return true;
}

bool NativeWindowDrm::UpdateCursor(const std::string& cursor_name, double x,
                                   double y) {
  if (cursor_name.compare(cursor_name_) == 0) {
    return true;
  }
  cursor_name_ = cursor_name;

  if (cursor_name.compare(kCursorNameNone) == 0) {
    return DismissCursor();
  }

  if (!CreateCursorBuffer(cursor_name)) {
    return false;
  }

  MoveCursor(x, y);
  auto result = drmModeSetCursor(drm_device_, drm_crtc_->crtc_id,
                                 gbm_bo_get_handle(gbm_cursor_bo_).u32,
                                 kCursorBufferWidth, kCursorBufferHeight);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to set cursor buffer. (" << result << ")";
    return false;
  }
  return true;
}

bool NativeWindowDrm::DismissCursor() {
  auto result = drmModeSetCursor(drm_device_, drm_crtc_->crtc_id, 0, 0, 0);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to set cursor buffer. (" << result << ")";
    return false;
  }
  return true;
}

bool NativeWindowDrm::MoveCursor(double x, double y) {
  auto result =
      drmModeMoveCursor(drm_device_, drm_crtc_->crtc_id,
                        x - cursor_hotspot_.first, y - cursor_hotspot_.second);
  if (result < 0) {
    LINUXES_LOG(ERROR) << "Could not move the mouse cursor: " << result;
    return false;
  }
  return true;
}

bool NativeWindowDrm::ConfigureDisplay() {
  auto resources = drmModeGetResources(drm_device_);
  if (!resources) {
    LINUXES_LOG(ERROR) << "Couldn't get resources";
  }
  auto connector = FindConnector(resources);
  if (!connector) {
    LINUXES_LOG(ERROR) << "Couldn't find a connector";
    return false;
  }

  drm_connector_id_ = connector->connector_id;
  drm_mode_info_ = connector->modes[0];
  LINUXES_LOG(INFO) << "resolution: " << drm_mode_info_.hdisplay << "x"
                    << drm_mode_info_.vdisplay;

  auto* encoder = FindEncoder(resources, connector);
  if (!encoder) {
    LINUXES_LOG(ERROR) << "Couldn't find a connector";
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

bool NativeWindowDrm::CreateCursorBuffer(const std::string& cursor_name) {
  if (!gbm_cursor_bo_) {
    gbm_cursor_bo_ = gbm_bo_create(gbm_device_, kCursorBufferWidth,
                                   kCursorBufferHeight, GBM_BO_FORMAT_ARGB8888,
                                   GBM_BO_USE_CURSOR | GBM_BO_USE_WRITE);
    if (!gbm_cursor_bo_) {
      LINUXES_LOG(ERROR) << "Failed to create cursor buffer";
      return false;
    }
  }

  auto cursor_data = GetCursorData(cursor_name);
  uint32_t buf[kCursorBufferWidth * kCursorBufferHeight] = {0};
  for (int i = 0; i < kCursorHeight; i++) {
    memcpy(buf + i * kCursorBufferWidth, cursor_data + i * kCursorWidth,
           kCursorWidth * sizeof(uint32_t));
  }

  auto result = gbm_bo_write(gbm_cursor_bo_, buf, sizeof(buf));
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to write cursor data. (" << result << ")";
    return false;
  }
  return true;
}

const uint32_t* NativeWindowDrm::GetCursorData(const std::string& cursor_name) {
  // If there is no cursor data corresponding to the Flutter's cursor name, it
  // is defined as empty. If empty, the default cursor data(LeftPtr) will be
  // displayed.
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