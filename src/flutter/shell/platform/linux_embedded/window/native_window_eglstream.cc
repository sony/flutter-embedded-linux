// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_eglstream.h"

#include <fcntl.h>
#include <sys/mman.h>
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

NativeWindowEglstream::NativeWindowEglstream(const char* deviceFilename) {
  drm_device_ = open(deviceFilename, O_RDWR | O_CLOEXEC);
  if (drm_device_ == -1) {
    LINUXES_LOG(ERROR) << "Couldn't open " << deviceFilename;
    return;
  }

  ConfigureDisplay();

  valid_ = true;
}

NativeWindowEglstream::~NativeWindowEglstream() {
  if (drm_device_ == -1) {
    return;
  }

  if (drm_crtc_) {
    drmModeSetCrtc(drm_device_, drm_crtc_->crtc_id, drm_crtc_->buffer_id,
                   drm_crtc_->x, drm_crtc_->y, &drm_connector_id_, 1,
                   &drm_crtc_->mode);
    drmModeFreeCrtc(drm_crtc_);
  }

  if (drm_property_blob_) {
    drmModeDestroyPropertyBlob(drm_device_, drm_property_blob_);
  }

  if (drm_fb_) {
    drmModeRmFB(drm_device_, drm_fb_);
  }

  close(drm_device_);
}

bool NativeWindowEglstream::Resize(const size_t width,
                                   const size_t height) const {
  if (!valid_) {
    LINUXES_LOG(ERROR) << "Failed to resize the window.";
    return false;
  }

  // todo: implement here.
  LINUXES_LOG(ERROR) << "TODO: implement here!!";

  return false;
}

bool NativeWindowEglstream::ShowCursor(double x, double y) {
  // todo: implement here.
  return true;
}

bool NativeWindowEglstream::UpdateCursor(const std::string& cursor_name,
                                         double x, double y) {
  if (cursor_name.compare(cursor_name_) == 0) {
    return true;
  }
  cursor_name_ = cursor_name;

  if (cursor_name.compare(kCursorNameNone) == 0) {
    return DismissCursor();
  }

  // todo: implement here.
  return true;
}

bool NativeWindowEglstream::DismissCursor() {
  // todo: implement here.
  return true;
}

bool NativeWindowEglstream::MoveCursor(double x, double y) {
  auto result =
      drmModeMoveCursor(drm_device_, drm_crtc_->crtc_id,
                        x - cursor_hotspot_.first, y - cursor_hotspot_.second);
  if (result < 0) {
    LINUXES_LOG(ERROR) << "Could not move the mouse cursor: " << result;
    return false;
  }
  return true;
}

bool NativeWindowEglstream::ConfigureDisplay() {
  if (SetDrmClientCapability() != 0) {
    LINUXES_LOG(ERROR) << "Couldn't set drm client capability";
    return false;
  }

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

  auto plane_resources = drmModeGetPlaneResources(drm_device_);
  if (!plane_resources) {
    LINUXES_LOG(ERROR) << "Couldn't get plane resources";
    return false;
  }
  drm_plane_id_ = FindPlane(plane_resources);
  drmModeFreePlaneResources(plane_resources);
  if (drm_plane_id_ == -1) {
    LINUXES_LOG(ERROR) << "Could not find a plane.";
    return false;
  }

  auto atomic = drmModeAtomicAlloc();
  if (!atomic) {
    LINUXES_LOG(ERROR) << "Couldn't allocate atomic";
    return false;
  }
  int ret = -1;
  if (AssignAtomicRequest(atomic)) {
    ret = drmModeAtomicCommit(drm_device_, atomic,
                              DRM_MODE_ATOMIC_ALLOW_MODESET, nullptr);
  }
  drmModeAtomicFree(atomic);
  if (ret != 0) {
    LINUXES_LOG(ERROR) << "Failed to commit an atomic property change request";
    return false;
  }

  return true;
}

int NativeWindowEglstream::SetDrmClientCapability() {
  if (drmSetClientCap(drm_device_, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) == 0) {
    return drmSetClientCap(drm_device_, DRM_CLIENT_CAP_ATOMIC, 1);
  }
  return -1;
}

drmModeConnectorPtr NativeWindowEglstream::FindConnector(
    drmModeResPtr resources) {
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

drmModeEncoder* NativeWindowEglstream::FindEncoder(
    drmModeRes* resources, drmModeConnector* connector) {
  if (connector->encoder_id) {
    return drmModeGetEncoder(drm_device_, connector->encoder_id);
  }
  // no encoder found
  return nullptr;
}

uint32_t NativeWindowEglstream::FindPlane(drmModePlaneResPtr resources) {
  for (uint32_t i = 0; i < resources->count_planes; i++) {
    auto plane = drmModeGetPlane(drm_device_, resources->planes[i]);
    if (plane) {
      drmModeFreePlane(plane);

      constexpr char kPropNamePlaneType[] = "type";
      auto type = GetPropertyValue(resources->planes[i], DRM_MODE_OBJECT_PLANE,
                                   kPropNamePlaneType);
      if (type == DRM_PLANE_TYPE_PRIMARY) {
        return resources->planes[i];
      }
    }
  }
  // no plane found
  return -1;
}

uint64_t NativeWindowEglstream::GetPropertyValue(uint32_t id, uint32_t type,
                                                 const char* prop_name) {
  uint64_t value = -1;
  auto properties = drmModeObjectGetProperties(drm_device_, id, type);
  if (properties) {
    for (uint32_t i = 0; i < properties->count_props; i++) {
      auto property = drmModeGetProperty(drm_device_, properties->props[i]);
      if (property) {
        if (strcmp(prop_name, property->name) == 0) {
          value = properties->prop_values[i];
          drmModeFreeProperty(property);
          break;
        }
        drmModeFreeProperty(property);
      }
    }
    drmModeFreeObjectProperties(properties);
  }
  return value;
}

bool NativeWindowEglstream::AssignAtomicRequest(drmModeAtomicReqPtr atomic) {
  if (!CreatePropertyBlob() || !CreateFb()) {
    return false;
  }

  struct drm_property_ids property_ids = {0};
  GetPropertyIds(&property_ids);
  drmModeAtomicAddProperty(atomic, drm_crtc_->crtc_id,
                           property_ids.crtc.mode_id, drm_property_blob_);
  drmModeAtomicAddProperty(atomic, drm_crtc_->crtc_id, property_ids.crtc.active,
                           1);
  drmModeAtomicAddProperty(atomic, drm_connector_id_,
                           property_ids.connector.crtc_id, drm_crtc_->crtc_id);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.src_x, 0);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.src_y, 0);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.src_w,
                           drm_mode_info_.hdisplay << 16);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.src_h,
                           drm_mode_info_.vdisplay << 16);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.crtc_x, 0);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.crtc_y, 0);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.crtc_w,
                           drm_mode_info_.hdisplay);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.crtc_h,
                           drm_mode_info_.vdisplay);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.fb_id,
                           drm_fb_);
  drmModeAtomicAddProperty(atomic, drm_plane_id_, property_ids.plane.crtc_id,
                           drm_crtc_->crtc_id);
  return true;
}

void NativeWindowEglstream::GetPropertyIds(
    struct NativeWindowEglstream::drm_property_ids* property_ids) {
  struct drm_property_address crtc_table[] = {
      {"MODE_ID", &property_ids->crtc.mode_id},
      {"ACTIVE", &property_ids->crtc.active},
  };
  struct drm_property_address plane_table[] = {
      {"SRC_X", &property_ids->plane.src_x},
      {"SRC_Y", &property_ids->plane.src_y},
      {"SRC_W", &property_ids->plane.src_w},
      {"SRC_H", &property_ids->plane.src_h},
      {"CRTC_X", &property_ids->plane.crtc_x},
      {"CRTC_Y", &property_ids->plane.crtc_y},
      {"CRTC_W", &property_ids->plane.crtc_w},
      {"CRTC_H", &property_ids->plane.crtc_h},
      {"FB_ID", &property_ids->plane.fb_id},
      {"CRTC_ID", &property_ids->plane.crtc_id},
      {"CRTC_ID", &property_ids->connector.crtc_id},
  };
  struct drm_property_address connector_table[] = {
      {"CRTC_ID", &property_ids->connector.crtc_id},
  };

  GetPropertyAddress(drm_crtc_->crtc_id, DRM_MODE_OBJECT_CRTC, crtc_table,
                     sizeof(crtc_table) / sizeof(drm_property_address));
  GetPropertyAddress(drm_plane_id_, DRM_MODE_OBJECT_PLANE, plane_table,
                     sizeof(plane_table) / sizeof(drm_property_address));
  GetPropertyAddress(drm_connector_id_, DRM_MODE_OBJECT_CONNECTOR,
                     connector_table,
                     sizeof(connector_table) / sizeof(drm_property_address));
}

void NativeWindowEglstream::GetPropertyAddress(
    uint32_t id, uint32_t type,
    NativeWindowEglstream::drm_property_address* table, size_t length) {
  auto properties = drmModeObjectGetProperties(drm_device_, id, type);
  if (properties) {
    for (uint32_t i = 0; i < properties->count_props; i++) {
      auto property = drmModeGetProperty(drm_device_, properties->props[i]);
      if (property) {
        for (uint32_t j = 0; j < length; j++) {
          if (strcmp(table[j].name, property->name) == 0) {
            *(table[j].ptr) = property->prop_id;
            break;
          }
        }
      }
      drmModeFreeProperty(property);
    }
    drmModeFreeObjectProperties(properties);
  }
}

bool NativeWindowEglstream::CreatePropertyBlob() {
  if (drmModeCreatePropertyBlob(drm_device_, &drm_mode_info_,
                                sizeof(drm_mode_info_),
                                &drm_property_blob_) != 0) {
    LINUXES_LOG(ERROR) << "Failed to create property blob";
    return false;
  }
  return true;
}

bool NativeWindowEglstream::CreateFb() {
  struct drm_mode_create_dumb create_dump = {0};
  create_dump.width = drm_mode_info_.hdisplay;
  create_dump.height = drm_mode_info_.vdisplay;
  create_dump.bpp = 32;
  if (drmIoctl(drm_device_, DRM_IOCTL_MODE_CREATE_DUMB, &create_dump) != 0) {
    LINUXES_LOG(ERROR) << "Failed to create dumb buffer";
    return false;
  }

  if (drmModeAddFB(drm_device_, drm_mode_info_.hdisplay,
                   drm_mode_info_.vdisplay, 24, 32, create_dump.pitch,
                   create_dump.handle, &drm_fb_) != 0) {
    LINUXES_LOG(ERROR) << "Failed to add a framebuffer";
    return false;
  }

  struct drm_mode_map_dumb map_dump = {0};
  map_dump.handle = create_dump.handle;
  if (drmIoctl(drm_device_, DRM_IOCTL_MODE_MAP_DUMB, &map_dump) != 0) {
    LINUXES_LOG(ERROR) << "Failed to map dumb buffer";
    return false;
  }

  auto map =
      static_cast<uint8_t*>(mmap(0, create_dump.size, PROT_READ | PROT_WRITE,
                                 MAP_SHARED, drm_device_, map_dump.offset));
  if (map == MAP_FAILED) {
    LINUXES_LOG(ERROR) << "Failed to mmap fb";
    return false;
  }

  memset(map, 0, create_dump.size);
  return true;
}

const uint32_t* NativeWindowEglstream::GetCursorData(
    const std::string& cursor_name) {
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