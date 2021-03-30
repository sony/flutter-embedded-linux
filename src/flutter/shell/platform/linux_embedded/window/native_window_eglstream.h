// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_EGLSTREAM_H_

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>

#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

class NativeWindowEglstream : public NativeWindow<void> {
 public:
  NativeWindowEglstream(const char* deviceFilename);
  ~NativeWindowEglstream();

  // |NativeWindow|
  bool Resize(const size_t width, const size_t height) const override;

  int* DrmDevice() { return &drm_device_; }

  uint32_t PlaneId() { return drm_plane_id_; }

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
  struct drm_property_ids {
    struct {
      uint32_t mode_id;
      uint32_t active;
    } crtc;

    struct {
      uint32_t src_x;
      uint32_t src_y;
      uint32_t src_w;
      uint32_t src_h;
      uint32_t crtc_x;
      uint32_t crtc_y;
      uint32_t crtc_w;
      uint32_t crtc_h;
      uint32_t fb_id;
      uint32_t crtc_id;
    } plane;

    struct {
      uint32_t crtc_id;
    } connector;
  };

  struct drm_property_address {
    const char* name;
    uint32_t* ptr;
  };

  bool ConfigureDisplay();

  int SetDrmClientCapability();

  drmModeConnectorPtr FindConnector(drmModeResPtr resources);

  drmModeEncoder* FindEncoder(drmModeRes* resources,
                              drmModeConnector* connector);

  uint32_t FindPlane(drmModePlaneResPtr resources);

  uint64_t GetPropertyValue(uint32_t id, uint32_t type, const char* prop_name);

  bool AssignAtomicRequest(drmModeAtomicReqPtr atomic);

  void GetPropertyIds(struct drm_property_ids* property_ids);

  void GetPropertyAddress(uint32_t id, uint32_t type,
                          drm_property_address* table, size_t length);

  bool CreatePropertyBlob();

  bool CreateFb();

  // Convert Flutter's cursor value to cursor data.
  const uint32_t* GetCursorData(const std::string& cursor_name);

  int drm_device_;
  uint32_t drm_connector_id_;
  drmModeCrtc* drm_crtc_ = nullptr;
  drmModeModeInfo drm_mode_info_;
  uint32_t drm_plane_id_;
  uint32_t drm_property_blob_ = 0;
  uint32_t drm_fb_ = 0;

  std::string cursor_name_ = "";
  std::pair<int32_t, int32_t> cursor_hotspot_ = {0, 0};
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_EGLSTREAM_H_