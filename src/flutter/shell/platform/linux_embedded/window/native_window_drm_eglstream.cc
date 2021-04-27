// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_drm_eglstream.h"

#include <sys/mman.h>

#include <cstring>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/context_egl_stream.h"
#include "flutter/shell/platform/linux_embedded/surface/cursor_data.h"

namespace flutter {

namespace {
constexpr char kCursorNameNone[] = "none";
}  // namespace

NativeWindowDrmEglstream::NativeWindowDrmEglstream(const char* device_filename)
    : NativeWindowDrm(device_filename) {
  if (!valid_) {
    return;
  }

  valid_ = ConfigureDisplayAdditional();

  // drmIsMaster() is a relatively new API, and the main target of EGLStream is
  // NVIDIA devices. Currently, NVIDIA devices' libdrm is a bit older.
  // drmIsMaster() may not exist. However, according to NVIDA's API document,
  // clients is always granted to become DRM master. So, we don't need to check
  // permissions with it.
  //
  // See also:
  // https://docs.nvidia.com/drive/nvvib_docs/NVIDIA%20DRIVE%20Linux%20SDK%20Development%20Guide/baggage/group__direct__rendering__manager.html#ga3f9326af9fc8eddc23dc6e263a2160a1
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

std::unique_ptr<SurfaceGl> NativeWindowDrmEglstream::CreateRenderSurface() {
  return std::make_unique<SurfaceGl>(std::make_unique<ContextEglStream>(
      std::make_unique<EnvironmentEglStream>()));
}

bool NativeWindowDrmEglstream::ConfigureDisplayAdditional() {
  if (!SetDrmClientCapabilities()) {
    LINUXES_LOG(ERROR) << "Couldn't set drm client capability";
    return false;
  }

  auto plane_resources = drmModeGetPlaneResources(drm_device_);
  if (!plane_resources) {
    LINUXES_LOG(ERROR) << "Couldn't get plane resources";
    return false;
  }
  drm_plane_id_ = FindPrimaryPlaneId(plane_resources);
  drmModeFreePlaneResources(plane_resources);
  if (drm_plane_id_ == -1) {
    LINUXES_LOG(ERROR) << "Couldn't find a plane.";
    return false;
  }

  auto atomic = drmModeAtomicAlloc();
  if (!atomic) {
    LINUXES_LOG(ERROR) << "Couldn't allocate atomic";
    return false;
  }
  int result = -1;
  if (AssignAtomicRequest(atomic)) {
    result = drmModeAtomicCommit(drm_device_, atomic,
                                 DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
  }
  drmModeAtomicFree(atomic);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to commit an atomic property change request";
    return false;
  }

  return true;
}

bool NativeWindowDrmEglstream::SetDrmClientCapabilities() {
  if (drmSetClientCap(drm_device_, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) != 0) {
    LINUXES_LOG(ERROR) << "Couldn't set DRM_CLIENT_CAP_UNIVERSAL_PLANES";
    return false;
  }
  if (drmSetClientCap(drm_device_, DRM_CLIENT_CAP_ATOMIC, 1) != 0) {
    LINUXES_LOG(ERROR) << "Couldn't set DRM_CLIENT_CAP_ATOMIC";
    return false;
  }
  return true;
}

uint32_t NativeWindowDrmEglstream::FindPrimaryPlaneId(
    drmModePlaneResPtr resources) {
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
        if (std::strcmp(prop_name, property->name) == 0) {
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
  if (drmModeCreatePropertyBlob(drm_device_, &drm_mode_info_,
                                sizeof(drm_mode_info_),
                                &drm_property_blob_) != 0) {
    LINUXES_LOG(ERROR) << "Failed to create property blob";
    return false;
  }

  // Set the crtc mode and activate.
  NativeWindowDrmEglstream::DrmProperty crtc_table[] = {
      {"MODE_ID", drm_property_blob_},
      {"ACTIVE", 1},
  };
  if (!AssignAtomicPropertyValue(atomic, drm_crtc_->crtc_id,
                                 DRM_MODE_OBJECT_CRTC, crtc_table)) {
    return false;
  }

  // Set the connector.
  NativeWindowDrmEglstream::DrmProperty connector_table[] = {
      {"CRTC_ID", drm_crtc_->crtc_id},
  };
  if (!AssignAtomicPropertyValue(atomic, drm_connector_id_,
                                 DRM_MODE_OBJECT_CONNECTOR, connector_table)) {
    return false;
  }

  // Set the plane source position, plane destination position, and crtc to
  // connect plane.
  NativeWindowDrmEglstream::DrmProperty plane_table[] = {
      {"SRC_X", 0},
      {"SRC_Y", 0},
      {"SRC_W", static_cast<uint64_t>(drm_mode_info_.hdisplay << 16)},
      {"SRC_H", static_cast<uint64_t>(drm_mode_info_.vdisplay << 16)},
      {"CRTC_X", 0},
      {"CRTC_Y", 0},
      {"CRTC_W", static_cast<uint64_t>(drm_mode_info_.hdisplay)},
      {"CRTC_H", static_cast<uint64_t>(drm_mode_info_.vdisplay)},
      {"CRTC_ID", drm_crtc_->crtc_id},
  };
  if (!AssignAtomicPropertyValue(atomic, drm_plane_id_, DRM_MODE_OBJECT_PLANE,
                                 plane_table)) {
    return false;
  }

  return true;
}

template <size_t N>
bool NativeWindowDrmEglstream::AssignAtomicPropertyValue(
    drmModeAtomicReqPtr atomic, uint32_t id, uint32_t type,
    NativeWindowDrmEglstream::DrmProperty (&table)[N]) {
  auto properties = drmModeObjectGetProperties(drm_device_, id, type);
  if (properties) {
    for (uint32_t i = 0; i < properties->count_props; i++) {
      auto property = drmModeGetProperty(drm_device_, properties->props[i]);
      if (property) {
        for (uint32_t j = 0; j < N; j++) {
          if (std::strcmp(table[j].name, property->name) == 0) {
            if (drmModeAtomicAddProperty(atomic, id, property->prop_id,
                                         table[j].value) < 0) {
              LINUXES_LOG(ERROR)
                  << "Failed to add " << table[j].name << " property";
              drmModeFreeProperty(property);
              drmModeFreeObjectProperties(properties);
              return false;
            }
            break;
          }
        }
      }
      drmModeFreeProperty(property);
    }
    drmModeFreeObjectProperties(properties);
  }
  return true;
}

}  // namespace flutter
