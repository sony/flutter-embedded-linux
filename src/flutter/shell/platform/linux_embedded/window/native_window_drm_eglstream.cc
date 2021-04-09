// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_drm_eglstream.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/cursor_data.h"

namespace flutter {

NativeWindowDrmEglstream::NativeWindowDrmEglstream(const char* deviceFilename) {
  drm_device_ = open(deviceFilename, O_RDWR | O_CLOEXEC);
  if (drm_device_ == -1) {
    LINUXES_LOG(ERROR) << "Couldn't open " << deviceFilename;
    return;
  }

  if (!ConfigureDisplay() || !ConfigureDisplayForEglstream()) {
    return;
  }

  valid_ = true;
}

NativeWindowDrmEglstream::~NativeWindowDrmEglstream() {
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

bool NativeWindowDrmEglstream::ShowCursor(double x, double y) {
  // todo: implement here.
  return true;
}

bool NativeWindowDrmEglstream::UpdateCursor(const std::string& cursor_name,
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

bool NativeWindowDrmEglstream::DismissCursor() {
  // todo: implement here.
  return true;
}

std::unique_ptr<SurfaceGlDrm<uint32_t, ContextEglDrmEglstream>>
NativeWindowDrmEglstream::CreateRenderSurface() {
  return std::make_unique<SurfaceGlDrm<uint32_t, ContextEglDrmEglstream>>(
      std::make_unique<ContextEglDrmEglstream>(
          std::make_unique<EnvironmentEglDrmEglstream>()));
}

bool NativeWindowDrmEglstream::ConfigureDisplayForEglstream() {
  if (SetDrmClientCapability() != 0) {
    LINUXES_LOG(ERROR) << "Couldn't set drm client capability";
    return false;
  }

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

int NativeWindowDrmEglstream::SetDrmClientCapability() {
  if (drmSetClientCap(drm_device_, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) == 0) {
    return drmSetClientCap(drm_device_, DRM_CLIENT_CAP_ATOMIC, 1);
  }
  return -1;
}

uint32_t NativeWindowDrmEglstream::FindPlane(drmModePlaneResPtr resources) {
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

uint64_t NativeWindowDrmEglstream::GetPropertyValue(uint32_t id, uint32_t type,
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

bool NativeWindowDrmEglstream::AssignAtomicRequest(drmModeAtomicReqPtr atomic) {
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

void NativeWindowDrmEglstream::GetPropertyIds(
    struct NativeWindowDrmEglstream::drm_property_ids* property_ids) {
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

void NativeWindowDrmEglstream::GetPropertyAddress(
    uint32_t id, uint32_t type,
    NativeWindowDrmEglstream::drm_property_address* table, size_t length) {
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

bool NativeWindowDrmEglstream::CreatePropertyBlob() {
  if (drmModeCreatePropertyBlob(drm_device_, &drm_mode_info_,
                                sizeof(drm_mode_info_),
                                &drm_property_blob_) != 0) {
    LINUXES_LOG(ERROR) << "Failed to create property blob";
    return false;
  }
  return true;
}

bool NativeWindowDrmEglstream::CreateFb() {
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

}  // namespace flutter