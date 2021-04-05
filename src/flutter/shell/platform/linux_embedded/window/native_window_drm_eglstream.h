// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>

#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"

namespace flutter {

class NativeWindowDrmEglstream : public NativeWindowDrm<uint32_t> {
 public:
  NativeWindowDrmEglstream(const char* deviceFilename);
  ~NativeWindowDrmEglstream();

  // |NativeWindowDrm|
  bool ShowCursor(double x, double y) override;

  // |NativeWindowDrm|
  bool UpdateCursor(const std::string& cursor_name, double x,
                    double y) override;

  // |NativeWindowDrm|
  bool DismissCursor() override;

  uint32_t PlaneId() { return drm_plane_id_; }

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

  uint32_t FindPlane(drmModePlaneResPtr resources);

  uint64_t GetPropertyValue(uint32_t id, uint32_t type, const char* prop_name);

  bool AssignAtomicRequest(drmModeAtomicReqPtr atomic);

  void GetPropertyIds(struct drm_property_ids* property_ids);

  void GetPropertyAddress(uint32_t id, uint32_t type,
                          drm_property_address* table, size_t length);

  bool CreatePropertyBlob();

  bool CreateFb();

  uint32_t drm_plane_id_;
  uint32_t drm_property_blob_ = 0;
  uint32_t drm_fb_ = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_